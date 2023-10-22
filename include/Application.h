#pragma once

#include "../include/Rendering/Renderer.h"
#include "../include/Fluid/Fluid.h"

#include <string>

enum ApplicationState
{
    RUNNING,
    EXIT
};

class Application
{
public:
    Application(std::string windowTitle, int windowWidth, int windowHeight);
    ~Application();

    int run();

private:
    std::string windowTitle;
    int windowWidth;
    int windowHeight;

    ApplicationState state = ApplicationState::RUNNING;

    int init();
    void destroy();

    Rendering::Renderer *renderer;

    Fluid::FluidOptions options;
    Fluid::Fluid *fluid;

    void update(float dt);
    void render(bool clear = true);

    bool enablePerPixelDensity = false;
    void renderPerPixelDensity(unsigned int skip);

    bool paused = true;
    bool stepSimulation = false;
    std::string selectedOption = "";
    void addSimulationControls();

    glm::vec2 mousePos;
    bool leftMouseDown = false;
    Fluid::FluidAttractor *attractor = nullptr;

    void createFluidInteractionListener();

    void createGui();
};