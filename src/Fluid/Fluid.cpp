#include "../../include/Fluid/Fluid.h"

#include <glm/glm.hpp>
#include <math.h>
#include <iostream>
#include <numbers>

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
        p->mass = options.particleMass;

        p->density = 0;
        p->pressure = 0;
        p->pressureForce = glm::vec2(0, 0);
        p->viscosityForce = glm::vec2(0, 0);

        particles.push_back(p);
    }
}

void Fluid::Fluid::update(float dt)
{
    updateGrid();

    // solve

    // precompute neighbours
    for (auto p : particles)
    {
        p->neighbours = getParticlesOfInfluence(p);
        // std::cout << p->neighbours.size() << std::endl;
    }

    solveDensity();
    solvePressure();
    solvePressureForce();
    solveViscosityForce();

    // apply forces
    applyGravity(dt);
    applySPHForces(dt);
    applyVelocity(dt);
    applyBoundingBox();
}

std::vector<Fluid::Particle *> &Fluid::Fluid::getParticles()
{
    return particles;
}

void Fluid::Fluid::clearParticles()
{
    for (auto p : particles)
    {
        delete p;
    }

    particles.clear();
}

Fluid::Grid &Fluid::Fluid::getGrid()
{
    return grid;
}

void Fluid::Fluid::applyGravity(float dt)
{
    for (auto p : particles)
    {
        p->velocity += options.gravity * dt;
    }
}

void Fluid::Fluid::applySPHForces(float dt)
{
    for (auto p : particles)
    {
        if (p->density == 0 || p->pressure == 0)
        {
            // std::cout << "density is 0" << std::endl;
            continue;
        }

        p->velocity += ((p->pressureForce + p->viscosityForce) / p->density) * dt;

        // std::cout << p->pressure << std::endl;
        // std::cout << p->density << std::endl;
        // std::cout << p->pressureForce.x << ", " << p->pressureForce.y << std::endl;
        // std::cout << p->viscosityForce.x << ", " << p->viscosityForce.y << std::endl;
    }
}

void Fluid::Fluid::applyVelocity(float dt)
{
    for (auto p : particles)
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

void Fluid::Fluid::solveDensity()
{
    for (auto p : particles)
    {
        p->density = 0;

        for (auto q : p->neighbours)
        {
            p->density += q.particle->mass * smoothingKernelPoly6.calculate(&q.distance, options.smoothingRadius);
            // std::cout << q.distance.distance << std::endl;
        }
    }
}

void Fluid::Fluid::solvePressure()
{
    for (auto p : particles)
    {
        p->pressure = options.stiffness * (p->density - options.desiredRestDensity);
    }
}

void Fluid::Fluid::solvePressureForce()
{
    for (auto p : particles)
    {
        p->pressureForce = glm::vec2(0, 0);

        for (auto q : p->neighbours)
        {
            if (q.particle->density == 0)
                continue;

            p->pressureForce += q.particle->mass * ((p->pressure + q.particle->pressure) / (2 * q.particle->density)) * smoothingKernelSpiky.calculateGradient(&q.distance, options.smoothingRadius);
        }

        p->pressureForce *= -1;
    }
}

void Fluid::Fluid::solveViscosityForce()
{
    for (auto p : particles)
    {
        p->viscosityForce = glm::vec2(0, 0);

        for (auto q : p->neighbours)
        {
            if (q.particle->density == 0)
                continue;

            p->viscosityForce += q.particle->mass * ((q.particle->velocity - p->velocity) / q.particle->density) * smoothingKernelViscosity.calculateLaplacian(&q.distance, options.smoothingRadius);
        }

        p->viscosityForce *= options.viscosity;
    }
}

std::vector<Fluid::ParticleNeighbour> Fluid::Fluid::getParticlesOfInfluence(Particle *p)
{
    float smoothingRadiusSqr = options.smoothingRadius * options.smoothingRadius;
    auto &key = p->gridKey;

    std::vector<ParticleNeighbour> close;

    // add all particles in the same cell
    // no need to check if they're in range
    // since they're in the same cell
    // so must be within smoothing radius
    for (Particle *q : grid[key])
    {
        if (p == q)
            continue;

        close.push_back(ParticleNeighbour{
            q,
            ParticleDistance{
                glm::distance(p->position, q->position),
                glm::normalize(p->position - q->position),
            },
        });
    }

    for (int xOff = -1; xOff <= 1; ++xOff)
    {
        for (int yOff = -1; yOff <= 1; ++yOff)
        {
            // skip if we're in the same cell
            if (xOff == 0 && yOff == 0)
                continue;

            auto gridKey = std::make_pair(key.first + xOff, key.second + yOff);
            if (grid.count(gridKey) == 0)
                continue;

            for (Particle *q : grid[gridKey])
            {
                auto temp = p->position - q->position;
                if (glm::dot(temp, temp) < smoothingRadiusSqr)
                {
                    close.push_back(ParticleNeighbour{
                        q,
                        ParticleDistance{
                            glm::distance(p->position, q->position),
                            glm::normalize(temp),
                        },
                    });
                }
            }
        }
    }

    return close;
}

void Fluid::Fluid::updateGrid()
{
    grid.clear();

    for (Particle *p : particles)
    {
        insertIntoGrid(p);
    }
}

void Fluid::Fluid::insertIntoGrid(Particle *p)
{
    p->gridKey = getGridKey(p);

    if (grid.count(p->gridKey) == 0)
        grid[p->gridKey] = std::vector<Particle *>();

    grid[p->gridKey].push_back(p);
}

std::pair<int, int> Fluid::Fluid::getGridKey(Particle *p)
{
    int cellWidth = options.smoothingRadius;
    int cellHeight = options.smoothingRadius;

    int x = (p->position.x - options.boundingBox.min.x) / cellWidth;
    int y = (p->position.y - options.boundingBox.min.y) / cellHeight;

    return std::make_pair(x, y);
}