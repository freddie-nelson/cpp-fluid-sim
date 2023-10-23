#include "../../include/Fluid/Fluid.h"

#include <glm/glm.hpp>
#include <math.h>
#include <iostream>
#include <numbers>
#include <chrono>

Fluid::Fluid::Fluid(FluidOptions &options) : options(options)
{
}

Fluid::Fluid::~Fluid()
{
    for (auto p : particles)
    {
        delete p;
    }
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
        p->tensionForce = glm::vec2(0, 0);

        p->neighbours = std::vector<ParticleNeighbour>();

        particles.push_back(p);
    }
}

void Fluid::Fluid::update(float dt)
{

    // pre solve
    for (auto p : particles)
    {
        applyGravity(p, dt);

        // calculate predicted position
        if (options.usePredictedPositions)
            p->predictedPosition = p->position + p->velocity * dt;
    }

    // update grid
    // int start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    updateGrid(options.usePredictedPositions);
    // int end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << std::endl
    //           << "update grid: " << end - start << "ms" << std::endl;

    // start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    for (auto p : particles)
    {
        // update mass and radius
        p->mass = options.particleMass;
        p->radius = options.particleRadius;

        // precompute neighbours
        p->neighbours = getParticlesOfInfluence(p, options.usePredictedPositions);
    }
    // end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "get neighbours: " << end - start << "ms" << std::endl;

    // solve
    // start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    for (auto p : particles)
    {
        solveDensityPressure(p);
    }

    // solve forces
    for (auto p : particles)
    {
        solvePressureForce(p);
        solveViscosityForce(p);
        // solveTensionForce(p);
    }

    // end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "solve: " << end - start << "ms" << std::endl;

    // apply forces
    for (auto p : particles)
    {
        applySPHForces(p, dt);
        applyAttractors(p, dt);
        applyVelocity(p, dt);
        applyBoundingBox(p);
    }
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

void Fluid::Fluid::addAttractor(FluidAttractor *attractor)
{
    removeAttractor(attractor);
    attractors.push_back(attractor);
}

bool Fluid::Fluid::removeAttractor(FluidAttractor *attractor)
{
    for (int i = 0; i < attractors.size(); i++)
    {
        if (attractors[i] == attractor)
        {
            attractors.erase(attractors.begin() + i);
            return true;
        }
    }

    return false;
}

void Fluid::Fluid::clearAttractors()
{
    attractors.clear();
}

Fluid::Grid &Fluid::Fluid::getGrid()
{
    return grid;
}

void Fluid::Fluid::solveDensityPressure(Particle *p)
{
    p->density = 0;

    for (auto q : p->neighbours)
    {
        p->density += q.particle->mass * smoothingKernelPoly6.calculate(&q.distance, options.smoothingRadius);
    }

    p->pressure = options.stiffness * (p->density - options.desiredRestDensity);

    // limit pressure to 200
    // to try and prevent blow ups
    if (p->pressure > options.pressureLimit)
        p->pressure = options.pressureLimit;
}

float Fluid::Fluid::solveDensityAtPoint(const glm::vec2 &point)
{
    float density = 0;

    for (auto p : particles)
    {
        ParticleDistance pd;
        pd.direction = point - p->position;
        pd.distance = glm::length(pd.direction);
        pd.direction /= pd.distance;

        density += p->mass * smoothingKernelPoly6.calculate(&pd, options.smoothingRadius);
    }

    return density;
}

void Fluid::Fluid::solvePressureForce(Particle *p)
{
    p->pressureForce = glm::vec2(0, 0);

    for (auto q : p->neighbours)
    {
        float sharedPressure = (p->pressure + q.particle->pressure) / 2;
        float smoothing = smoothingKernelSpiky.calculateGradient(&q.distance, options.smoothingRadius);

        glm::vec2 pressureForce = sharedPressure * q.distance.direction * q.particle->mass / q.particle->density;

        p->pressureForce += pressureForce * smoothing;
        p->pressureNearForce += pressureForce * static_cast<float>(std::pow(smoothing, 4));
    }

    p->pressureForce *= -1;
    p->pressureNearForce *= -1;
}

void Fluid::Fluid::solveViscosityForce(Particle *p)
{
    p->viscosityForce = glm::vec2(0, 0);

    for (auto q : p->neighbours)
    {
        p->viscosityForce += (q.particle->velocity - p->velocity) * smoothingKernelPoly6.calculate(&q.distance, options.smoothingRadius);
    }

    p->viscosityForce *= options.viscosity;
}

void Fluid::Fluid::solveTensionForce(Particle *p)
{
    p->tensionForce = glm::vec2(0, 0);

    for (auto q : p->neighbours)
    {
        float colorFieldNoSmoothingKernel = q.particle->mass * (1 / q.particle->density);

        glm::vec2 n = colorFieldNoSmoothingKernel * smoothingKernelPoly6.calculateGradient(&q.distance, options.smoothingRadius) * q.distance.direction;
        float modN = glm::length(n);

        if (modN < options.surfaceTensionThreshold)
            continue;

        glm::vec2 normalizedN = n / modN;
        float colorFieldLaplacian = colorFieldNoSmoothingKernel * smoothingKernelPoly6.calculateLaplacian(&q.distance, options.smoothingRadius);

        p->tensionForce += -options.surfaceTension * colorFieldLaplacian * normalizedN;
    }
}

void Fluid::Fluid::applyGravity(Particle *p, float dt)
{
    p->velocity += options.gravity * dt;
}

void Fluid::Fluid::applySPHForces(Particle *p, float dt)
{
    if (p->density == 0)
    {
        return;
    }

    p->velocity += ((p->pressureForce + p->pressureNearForce + p->viscosityForce + p->tensionForce) / p->density) * dt;
}

void Fluid::Fluid::applyAttractors(Particle *p, float dt)
{
    for (auto a : attractors)
    {
        glm::vec2 pToA = a->position - p->position;
        float dist = glm::length(pToA);
        glm::vec2 dir = pToA / dist;

        auto pd = new ParticleDistance{
            dist,
            dir,
        };

        if (dist < a->radius)
        {
            p->velocity += -a->strength * smoothingKernelPoly6.calculateGradient(pd, a->radius) * dir * dt;
        }

        delete pd;
    }
}

void Fluid::Fluid::applyVelocity(Particle *p, float dt)
{
    p->position += p->velocity * dt;
}

void Fluid::Fluid::applyBoundingBox(Particle *p)
{
    if (p->position.x < options.boundingBox.min.x)
    {
        p->position.x = options.boundingBox.min.x;
        p->velocity.x *= -options.boudingBoxRestitution;
    }

    if (p->position.x > options.boundingBox.max.x)
    {
        p->position.x = options.boundingBox.max.x;
        p->velocity.x *= -options.boudingBoxRestitution;
    }

    if (p->position.y < options.boundingBox.min.y)
    {
        p->position.y = options.boundingBox.min.y;
        p->velocity.y *= -options.boudingBoxRestitution;
    }

    if (p->position.y > options.boundingBox.max.y)
    {
        p->position.y = options.boundingBox.max.y;
        p->velocity.y *= -options.boudingBoxRestitution;
    }
}

std::vector<Fluid::ParticleNeighbour> Fluid::Fluid::getParticlesOfInfluence(Particle *p, bool usePredictedPosition)
{
    float smoothingRadiusSqr = options.smoothingRadius * options.smoothingRadius;
    auto &key = p->gridKey;

    std::vector<ParticleNeighbour> &close = p->neighbours;
    close.clear();

    // brute force
    // for (auto q : particles)
    // {

    //     auto pPosition = usePredictedPosition ? p->predictedPosition : p->position;
    //     auto qPosition = usePredictedPosition ? q->predictedPosition : q->position;

    //     auto temp = pPosition - qPosition;
    //     auto len = glm::length(temp);

    //     if (p == q || len >= options.smoothingRadius)
    //         continue;

    //     close.push_back(ParticleNeighbour{
    //         q,
    //         ParticleDistance{
    //             len == 0 ? 1.0f : len,
    //             len == 0 ? randomDirection() : temp / len,
    //         },
    //     });
    // }
    // return close;

    // add all particles in the same cell
    // no need to check if they're in range
    // since they're in the same cell
    // so must be within smoothing radius
    for (Particle *q : grid[key])
    {
        if (p == q)
            continue;

        auto pPosition = usePredictedPosition ? p->predictedPosition : p->position;
        auto qPosition = usePredictedPosition ? q->predictedPosition : q->position;

        auto temp = pPosition - qPosition;
        auto len = glm::length(temp);

        close.push_back(ParticleNeighbour{
            q,
            ParticleDistance{
                len == 0 ? 1.0f : len,
                len == 0 ? randomDirection() : temp / len,
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
                auto pPosition = usePredictedPosition ? p->predictedPosition : p->position;
                auto qPosition = usePredictedPosition ? q->predictedPosition : q->position;
                auto temp = pPosition - qPosition;

                if (glm::dot(temp, temp) < smoothingRadiusSqr)
                {
                    auto len = glm::length(temp);

                    // std::cout << temp.x << ", " << temp.y << std::endl;
                    // std::cout << glm::dot(temp, temp) << std::endl;
                    // std::cout << glm::length(temp) << std::endl;

                    close.push_back(ParticleNeighbour{
                        q,
                        ParticleDistance{
                            len == 0 ? 1.0f : len,
                            len == 0 ? randomDirection() : temp / len,
                        },
                    });
                }
            }
        }
    }

    return close;
}

void Fluid::Fluid::updateGrid(bool usePredictedPositions)
{
    grid.clear();

    for (Particle *p : particles)
    {
        insertIntoGrid(p, usePredictedPositions);
    }
}

void Fluid::Fluid::insertIntoGrid(Particle *p, bool usePredictedPosition)
{
    p->gridKey = getGridKey(p, usePredictedPosition);

    if (grid.count(p->gridKey) == 0)
        grid[p->gridKey] = std::vector<Particle *>();

    grid[p->gridKey].push_back(p);
}

std::pair<int, int> Fluid::Fluid::getGridKey(Particle *p, bool usePredictedPosition)
{
    int cellWidth = options.smoothingRadius;
    int cellHeight = options.smoothingRadius;

    auto position = usePredictedPosition ? p->predictedPosition : p->position;
    int x = (position.x - options.boundingBox.min.x) / cellWidth;
    int y = (position.y - options.boundingBox.min.y) / cellHeight;

    return std::make_pair(x, y);
}

glm::vec2 Fluid::Fluid::randomDirection()
{
    // std::cout << "random direction" << std::endl;

    float angle = static_cast<float>(rand()) / RAND_MAX * 2 * std::numbers::pi;
    return glm::normalize(glm::vec2(std::cos(angle), std::sin(angle)));
}