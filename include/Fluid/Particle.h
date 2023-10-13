#pragma once

#include <glm/vec2.hpp>
#include <utility>
#include <vector>

namespace Fluid
{
    struct Particle;

    struct ParticleDistance
    {
        float distance;
        glm::vec2 direction;
    };

    struct ParticleNeighbour
    {
        Particle *particle;
        ParticleDistance distance;
    };

    struct Particle
    {
        glm::vec2 position;
        glm::vec2 velocity;
        float radius;
        float mass;

        float density;
        float pressure;
        glm::vec2 pressureForce;
        glm::vec2 viscosityForce;

        // cached neighbours
        std::vector<ParticleNeighbour> neighbours;

        // cached grid key
        std::pair<int, int> gridKey = std::make_pair(-1, -1);
    };
}