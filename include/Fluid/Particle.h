#pragma once

#include <glm/vec2.hpp>
#include <utility>

namespace Fluid
{
    struct Particle
    {
        glm::vec2 position;
        glm::vec2 velocity;
        float radius;

        // cached grid key
        std::pair<int, int> gridKey = std::make_pair(-1, -1);
    };
}