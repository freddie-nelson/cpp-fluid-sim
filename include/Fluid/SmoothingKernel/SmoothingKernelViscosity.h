#pragma once

#include "./SmoothingKernel.h"

namespace Fluid
{
    class SmoothingKernelViscosity : public SmoothingKernel
    {
    public:
        float calculate(ParticleDistance *distance, float smoothingRadius);
        glm::vec2 calculateGradient(ParticleDistance *distance, float smoothingRadius);
        float calculateLaplacian(ParticleDistance *distance, float smoothingRadius);
    };
}