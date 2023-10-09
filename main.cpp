#include "./include/Application.h"

int main(int argv, char **args)
{
    Application app("Fluid Sim", 1000, 600);
    return app.run();
}