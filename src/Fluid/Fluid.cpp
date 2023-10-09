#include "../../include/Fluid/Fluid.h"

#include <math.h>

Fluid::Fluid::Fluid(FluidOptions &options) : options(options)
{
}

Fluid::Fluid::~Fluid()
{
}

void Fluid::Fluid::init()
{
    // create particles
    clearParticles();

    int gridSize = std::sqrt(options.numParticles);
    int particleOffset = options.particleRadius * 2 + options.particleSpacing;
    float gridOffset = (gridSize - 1) * particleOffset * 0.5f;

    for (int i = 0; i < options.numParticles; i++)
    {
        Particle *p = new Particle();

        p->position = glm::vec2(i % gridSize, i / gridSize);
        p->position *= particleOffset;
        p->position += options.initialCentre;
        p->position -= glm::vec2(gridOffset, gridOffset);

        p->radius = options.particleRadius;

        particles.push_back(p);
    }
}

void Fluid::Fluid::update(float dt)
{
}

std::vector<Fluid::Particle *> &Fluid::Fluid::getParticles()
{
    return particles;
}

void Fluid::Fluid::clearParticles()
{
    for (Particle *p : particles)
    {
        delete p;
    }

    particles.clear();
}