#include "../include/Application.h"
// #include "../include/Rendering/SDLRenderer.h"
#include "../include/Rendering/SFMLRenderer.h"
#include "../include/Globals.h"
#include "../include/Utility/Timestep.h"

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
        update(dt);
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
    renderer = new Rendering::SFMLRenderer(windowTitle, windowWidth, windowHeight);
    if (renderer->init() != 0)
    {
        std::cout << "Failed to initialize renderer." << std::endl;
        return 1;
    }

    // init fluid
    options = Fluid::FluidOptions{
        numParticles : 400,
        particleRadius : 10,
        particleSpacing : 5,
        initialCentre : glm::vec2(windowWidth / 2, windowHeight / 2),

        gravity : glm::vec2(0, 120.0f),

        boundingBox : Fluid::AABB{
            min : glm::vec2(0, 0),
            max : glm::vec2(windowWidth, windowHeight)
        },
        boudingBoxRestitution : 0.7f,

        smoothingRadius : 100.0f,
        stiffness : 10000.0f,
        desiredRestDensity : 0.0001f,
        particleMass : 10.0f,
        viscosity : 1.0f
    };

    fluid = new Fluid::Fluid(options);
    fluid->init();

    return 0;
}

void Application::destroy()
{
    delete renderer;
    delete this;
}

void Application::update(float dt)
{
    fluid->update(dt);
}

void Application::render(bool clear)
{
    // clear last frame
    if (clear)
        renderer->clear();

    // draw particles
    for (auto p : fluid->getParticles())
    {
        renderer->circle(Rendering::Circle{p->position, p->radius},
                         Rendering::Color{0, 0, 255, 255});
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
