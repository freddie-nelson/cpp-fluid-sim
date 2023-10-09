#pragma once

#include <glm/vec2.hpp>

namespace Fluid
{
    struct Particle
    {
        glm::vec2 position;
        glm::vec2 velocity;
        float radius;
    };
}