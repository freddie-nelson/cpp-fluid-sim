#pragma once

#include "./SmoothingKernel.h"

namespace Fluid
{
    class SmoothingKernelSpiky : public SmoothingKernel
    {
    public:
        float calculate(ParticleDistance *distance, float smoothingRadius);
        float calculateGradient(ParticleDistance *distance, float smoothingRadius);
        float calculateLaplacian(ParticleDistance *distance, float smoothingRadius);
    };
}