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
        const bool shouldExit = renderer->pollEvents();
        if (shouldExit)
        {
            state = ApplicationState::EXIT;
            break;
        }

        // clear renderer first so that update functions can draw to the screen
        // renderer->clear();

        update(dt);
        render();

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
        numParticles : 20 * 20,
        particleRadius : 10,
        particleSpacing : 5,
        initialCentre : glm::vec2(windowWidth / 2, windowHeight / 2),

        gravity : glm::vec2(0, 150.0f),

        boundingBox : Fluid::AABB{
            min : glm::vec2(0, 0),
            max : glm::vec2(windowWidth, windowHeight)
        },
        boudingBoxRestitution : 0.8f
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

    // print timestep info
    std::cout << "\rdt: " << dt << "          ";
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

    // render
    renderer->present();
}
