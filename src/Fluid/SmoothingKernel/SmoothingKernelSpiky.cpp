#include "../../../include/Fluid/SmoothingKernel/SmoothingKernelSpiky.h"

#include <math.h>
#include <numbers>

float Fluid::SmoothingKernelSpiky::calculate(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0.0f || r >= h)
        return 0.0f;

    float volume = (std::numbers::pi * std::pow(h, 4)) / 6;
    return std::pow(h - r, 2) / volume;
}

glm::vec2 Fluid::SmoothingKernelSpiky::calculateGradient(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0.0f || r >= h)
        return glm::vec2(0, 0);

    float scale = 12 / (std::pow(h, 4) * std::numbers::pi);
    return (r - h) * scale * distance->direction;
}

float Fluid::SmoothingKernelSpiky::calculateLaplacian(ParticleDistance *distance, float smoothingRadius)
{
    return 0.0f;
}