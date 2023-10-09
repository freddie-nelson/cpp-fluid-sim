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

        p->velocity = glm::vec2(0, 0);
        p->radius = options.particleRadius;

        particles.push_back(p);
    }
}

void Fluid::Fluid::update(float dt)
{
    applyGravity(dt);
    applyVelocity(dt);
    applyBoundingBox();
}

void Fluid::Fluid::applyGravity(float dt)
{
    for (Particle *p : particles)
    {
        p->velocity += options.gravity * dt;
    }
}

void Fluid::Fluid::applyVelocity(float dt)
{
    for (Particle *p : particles)
    {
        p->position += p->velocity * dt;
    }
}

void Fluid::Fluid::applyBoundingBox()
{
    for (auto p : particles)
    {
        if (p->position.x - p->radius < options.boundingBox.min.x)
        {
            p->position.x = options.boundingBox.min.x + p->radius;
            p->velocity.x *= -options.boudingBoxRestitution;
        }

        if (p->position.x + p->radius > options.boundingBox.max.x)
        {
            p->position.x = options.boundingBox.max.x - p->radius;
            p->velocity.x *= -options.boudingBoxRestitution;
        }

        if (p->position.y - p->radius < options.boundingBox.min.y)
        {
            p->position.y = options.boundingBox.min.y + p->radius;
            p->velocity.y *= -options.boudingBoxRestitution;
        }

        if (p->position.y + p->radius > options.boundingBox.max.y)
        {
            p->position.y = options.boundingBox.max.y - p->radius;
            p->velocity.y *= -options.boudingBoxRestitution;
        }
    }
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