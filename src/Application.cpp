#include "../include/Application.h"
#include "../include/Rendering/Renderer.h"
#include "../include/Globals.h"
#include "../include/Utility/Timestep.h"
#include "../include/Utility/InputCodes.h"

#include <glm/glm.hpp>
#include <math.h>
#include <random>
#include <time.h>
#include <iostream>
#include <chrono>
#include <thread>

Application::Application(std::string windowTitle, int windowWidth, int windowHeight) : windowTitle(windowTitle), windowWidth(windowWidth), windowHeight(windowHeight)
{
}

Application::~Application()
{
    destroy();
}

int Application::run()
{
    const int initCode = init();
    if (initCode != 0)
    {
        return initCode;
    }

    const int desiredFps = 120;
    const int desiredFrameTime = 1000 / desiredFps;
    const float desiredDt = 1.0f / static_cast<float>(desiredFps);

    auto lastUpdateTime = timeSinceEpochMillisec() - desiredFrameTime;

    while (state == ApplicationState::RUNNING)
    {
        // update timestep
        const auto now = timeSinceEpochMillisec();
        const auto diff = now - lastUpdateTime;
        lastUpdateTime = now;

        // dt in seconds
        const float dt = (float)diff / 1000.0f;

        // wait for renderer events to be processed
        auto startTime = timeSinceEpochMillisec();
        const bool shouldExit = renderer->pollEvents();
        if (shouldExit)
        {
            state = ApplicationState::EXIT;
            break;
        }
        auto eventTime = timeSinceEpochMillisec() - startTime;

        // clear renderer first so that update functions can draw to the screen
        // renderer->clear();

        startTime = timeSinceEpochMillisec();
        update(desiredDt);
        auto updateTime = timeSinceEpochMillisec() - startTime;

        startTime = timeSinceEpochMillisec();
        render();
        auto renderTime = timeSinceEpochMillisec() - startTime;

        // print timestep info
        std::cout << "\rdt: " << dt
                  << " | events: " << eventTime << "ms"
                  << " | update: " << updateTime << "ms"
                  << " | render: " << renderTime << "ms"
                  << " | fps: " << 1.0f / dt << "        ";

        // wait until frame time is reached
        const auto frameEndTime = now + desiredFrameTime;
        while (timeSinceEpochMillisec() < frameEndTime)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }

    return 0;
}

int Application::init()
{
    // init renderer
    renderer = new Rendering::Renderer(windowTitle, windowWidth, windowHeight);
    if (renderer->init() != 0)
    {
        std::cout << "Failed to initialize renderer." << std::endl;
        return 1;
    }

    // init fluid
    options = Fluid::FluidOptions{
        numParticles : 900,
        particleRadius : 5,
        particleSpacing : 5,
        initialCentre : glm::vec2(windowWidth / 2, windowHeight / 2),

        gravity : glm::vec2(0, 1500.0f),

        boundingBox : Fluid::AABB{
            min : glm::vec2(0, 0),
            max : glm::vec2(windowWidth, windowHeight)
        },
        boudingBoxRestitution : 0.05f,

        smoothingRadius : 50.0f,
        stiffness : 1.0e6f,
        desiredRestDensity : 0.000025f,
        particleMass : 0.045f,
        viscosity : 0.13f,
        surfaceTension : 0.0f,
        surfaceTensionThreshold : 0.0f,

        usePredictedPositions : true,
    };

    fluid = new Fluid::Fluid(options);
    fluid->init();

    // add event listeners
    addSimulationControls();
    createFluidInteractionListener();

    // create gui
    // createGui();

    return 0;
}

void Application::destroy()
{
    delete renderer;
    delete this;
}

void Application::update(float dt)
{
    if (!paused || stepSimulation)
    {
        stepSimulation = false;
        fluid->update(dt);
    }
}

void Application::render(bool clear)
{
    // clear last frame
    if (clear)
        renderer->clear();

    if (enablePerPixelDensity)
    {
        renderPerPixelDensity(10);
    }

    // draw particles
    auto particles = fluid->getParticles();
    int numParticles = particles.size();

    Rendering::Circle circles[numParticles];
    Rendering::Color colors[numParticles];

    for (int i = 0; i < numParticles; i++)
    {
        auto p = particles[i];
        circles[i] = Rendering::Circle{p->position, options.particleRadius};
        colors[i] = getParticleColor(p);
    }

    renderer->shaderCircles(circles, colors, numParticles);

    // draw attractor
    if (isAttractorActive)
    {
        renderer->circle(Rendering::Circle{attractor->position, attractor->radius},
                         Rendering::Color{0, 255, 0, 255}, Rendering::RenderType::STROKE);
    }

    if (Globals::DEBUG_MODE)
    {
        // draw bounding box
        glm::vec2 bbPosition(options.boundingBox.min.x, options.boundingBox.min.y);
        float bbW = options.boundingBox.max.x - options.boundingBox.min.x;
        float bbH = options.boundingBox.max.y - options.boundingBox.min.y;

        renderer->rect(Rendering::Rect{bbPosition, bbW, bbH},
                       Rendering::Color{0, 255, 0, 255}, Rendering::RenderType::STROKE);

        // draw grid
        for (auto &kv : fluid->getGrid())
        {
            auto key = kv.first;
            auto particles = kv.second;

            glm::vec2 position(key.first * options.smoothingRadius, key.second * options.smoothingRadius);
            position += bbPosition;

            float w = options.smoothingRadius;
            float h = options.smoothingRadius;

            renderer->rect(Rendering::Rect{position, w, h},
                           Rendering::Color{255, 0, 0, 75}, Rendering::RenderType::STROKE);
        }

        // draw neighbours of particle 0
        auto p = fluid->getParticles()[0];
        auto neighbours = p->neighbours;
        // std::cout << "particle 0 neighbours count: " << neighbours.size() << std::endl;

        for (auto n : neighbours)
        {
            glm::vec2 nPosition = n.particle->position;
            nPosition += bbPosition;

            renderer->line(p->position, nPosition, Rendering::Color{255, 255, 255, 255});
        }
    }

    // render
    renderer->present();
}

Rendering::Color Application::getParticleColor(Fluid::Particle *particle)
{
    const float v = glm::dot(particle->velocity, particle->velocity);

    const float steps[] = {std::pow(60.0f, 2), std::pow(200.0f, 2), std::pow(400.0f, 2), std::pow(700.0f, 2)};
    const Rendering::Color colors[] = {{33, 55, 222, 255},
                                       {8, 177, 255, 255},
                                       {78, 255, 8, 255},
                                       {255, 53, 8, 255}};

    for (int i = 1; i < 4; i++)
    {
        if (v < steps[i])
        {
            float ratio = (v - steps[i - 1]) / (steps[i] - steps[i - 1]);
            Rendering::Color bg = colors[i - 1];
            bg.a = 255;

            Rendering::Color fg = colors[i];
            fg.a = 255 * ratio;

            return Rendering::blend(bg, fg);
        }
    }

    return colors[3];
}

void Application::renderPerPixelDensity(unsigned int skip)
{
    if (skip == 0)
        return;

    const Rendering::Color bg{255, 255, 255, 255};

    float maxDensity = options.desiredRestDensity * 2.0f;
    float minDensity = options.desiredRestDensity / 2.0f;

    for (int i = 0; i < windowWidth; i += skip)
    {
        for (int j = 0; j < windowHeight; j += skip)
        {
            glm::vec2 position(i, j);
            auto key = std::make_pair(i, j);

            float density = fluid->solveDensityAtPoint(position + glm::vec2(skip / 2.0f, skip / 2.0f));
            if (density == 0.0f)
                continue;

            float densityRatio = density / (density >= options.desiredRestDensity ? maxDensity : minDensity);
            float value = 255.0f * densityRatio;
            int valueInt = std::min(static_cast<int>(value), 255);

            Rendering::Color fg = density >= options.desiredRestDensity ? Rendering::Color{255, 0, 0, valueInt} : Rendering::Color{0, 0, 255, valueInt};
            auto c = Rendering::blend(bg, fg);
            // std::cout << c.r << ", " << c.g << ", " << c.b << ", " << c.a << std::endl;

            for (int px = 0; px < skip; px++)
            {
                for (int py = 0; py < skip; py++)
                {
                    renderer->pixel(position + glm::vec2(px, py), c);
                }
            }
        }
    }

    renderer->presentDrawnPixels();
}

void Application::addSimulationControls()
{
    renderer->on(Rendering::RendererEventType::KEY_UP,
                 [&](Rendering::RendererEvent event)
                 {
                     auto keyCode = *static_cast<Utility::KeyCode *>(event.data);

                     if (keyCode == Utility::KeyCode::KEY_SPACE)
                     {
                         paused = !paused;
                     }
                 });

    renderer->on(Rendering::RendererEventType::KEY_UP,
                 [&](Rendering::RendererEvent event)
                 {
                     auto keyCode = *static_cast<Utility::KeyCode *>(event.data);

                     if (keyCode == Utility::KeyCode::KEY_RIGHT)
                     {
                         stepSimulation = true;
                     }
                     else if (keyCode == Utility::KeyCode::KEY_R)
                     {
                         delete fluid;

                         fluid = new Fluid::Fluid(options);
                         fluid->init();
                     }
                     else if (keyCode == Utility::KeyCode::KEY_D)
                     {
                         Globals::DEBUG_MODE = !Globals::DEBUG_MODE;
                     }
                     else if (keyCode == Utility::KeyCode::KEY_C)
                     {
                         enablePerPixelDensity = !enablePerPixelDensity;
                     }
                     else if (keyCode == Utility::KeyCode::KEY_Y)
                     {
                         options.usePredictedPositions = !options.usePredictedPositions;
                     }
                     else if (keyCode == Utility::KeyCode::KEY_S)
                     {
                         std::cout << "[OPTION SELECTED]: stiffness" << std::endl;
                         selectedOption = "stiffness";
                     }
                     else if (keyCode == Utility::KeyCode::KEY_P)
                     {
                         std::cout << "[OPTION SELECTED]: particles" << std::endl;
                         selectedOption = "particles";
                     }
                     else if (keyCode == Utility::KeyCode::KEY_G)
                     {
                         std::cout << "[OPTION SELECTED]: gravity" << std::endl;
                         selectedOption = "gravity";
                     }
                     else if (keyCode == Utility::KeyCode::KEY_M)
                     {
                         std::cout << "[OPTION SELECTED]: particle mass" << std::endl;
                         selectedOption = "particle mass";
                     }
                     else if (keyCode == Utility::KeyCode::KEY_V)
                     {
                         std::cout << "[OPTION SELECTED]: viscosity" << std::endl;
                         selectedOption = "viscosity";
                     }
                     else if (keyCode == Utility::KeyCode::KEY_UP || keyCode == Utility::KeyCode::KEY_DOWN)
                     {
                         int sign = keyCode == Utility::KeyCode::KEY_UP ? 1 : -1;

                         if (selectedOption == "stiffness")
                         {
                             options.stiffness *= sign == 1 ? 1.1 : 0.9;
                             std::cout << "[STIFFNESS]: " << options.stiffness << std::endl;
                         }
                         else if (selectedOption == "particles")
                         {
                             options.numParticles += 10 * sign;
                             std::cout << "[PARTICLES]: " << options.numParticles << std::endl;
                         }
                         else if (selectedOption == "gravity")
                         {
                             options.gravity.y += 10.0f * sign;
                             std::cout << "[GRAVITY]: " << options.gravity.y << std::endl;
                         }
                         else if (selectedOption == "particle mass")
                         {
                             options.particleMass *= sign == 1 ? 1.025 : 0.975;
                             std::cout << "[PARTICLE MASS]: " << options.particleMass << std::endl;
                         }
                         else if (selectedOption == "viscosity")
                         {
                             options.viscosity += 0.01f * sign;
                             std::cout << "[VISCOSITY]: " << options.viscosity << std::endl;
                         }
                     }
                 });
}

void Application::createFluidInteractionListener()
{
    float radius = 200.0f;
    float strength = options.stiffness * (options.stiffness * 0.025f);

    attractor = new Fluid::FluidAttractor{
        position : glm::vec2(0, 0),
        radius : radius,
        strength : strength,
    };

    renderer->on(Rendering::RendererEventType::MOUSE_DOWN,
                 [&, strength](Rendering::RendererEvent event)
                 {
                     auto mouseButton = *static_cast<Utility::MouseButton *>(event.data);

                     if (mouseButton == Utility::MouseButton::MOUSE_LEFT && !isAttractorActive)
                     {
                         isAttractorActive = true;
                         attractor->strength = strength;
                         fluid->addAttractor(attractor);
                     }
                     else if (mouseButton == Utility::MouseButton::MOUSE_RIGHT && !isAttractorActive)
                     {
                         isAttractorActive = true;
                         attractor->strength = -strength;
                         fluid->addAttractor(attractor);
                     }
                 });

    renderer->on(Rendering::RendererEventType::MOUSE_UP,
                 [&](Rendering::RendererEvent event)
                 {
                     auto mouseButton = *static_cast<Utility::MouseButton *>(event.data);

                     if (mouseButton == Utility::MouseButton::MOUSE_LEFT || mouseButton == Utility::MouseButton::MOUSE_RIGHT)
                     {
                         isAttractorActive = false;
                         fluid->removeAttractor(attractor);
                     }
                 });

    renderer->on(Rendering::RendererEventType::MOUSE_MOVE,
                 [&](Rendering::RendererEvent event)
                 {
                     mousePos = *static_cast<glm::vec2 *>(event.data);
                     attractor->position = mousePos;
                 });
}

void Application::createGui()
{
    int guiWidth = 200;
    int margin = 10;
    glm::vec2 guiPosition{windowWidth - guiWidth - margin, margin};

    // create labels and sliders for options
    glm::vec2 currPosition = guiPosition;

    renderer->createLabel("Smoothing Radius", currPosition, glm::vec2(guiWidth, 30));
    currPosition.y += 30;

    renderer->createSlider(currPosition, glm::vec2(guiWidth, 30), 0.0f, 250.0f, options.smoothingRadius,
                           [&](float value)
                           {
                               options.smoothingRadius = value;
                           });
}