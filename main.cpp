#include "./include/Application.h"

int main(int argv, char **args)
{
    Application app("Fluid Sim", 1400, 1000);
    return app.run();
}