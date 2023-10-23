#pragma once

#include "../Particle.h"

namespace Fluid
{
    class SmoothingKernel
    {
    public:
        virtual float calculate(ParticleDistance *distance, float smoothingRadius) = 0;
        virtual float calculateGradient(ParticleDistance *distance, float smoothingRadius) = 0;
        virtual float calculateLaplacian(ParticleDistance *distance, float smoothingRadius) = 0;
    };
}