#include "../../../include/Fluid/SmoothingKernel/SmoothingKernelPoly6.h"

#include <math.h>
#include <numbers>

float Fluid::SmoothingKernelPoly6::calculate(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    // if (r <= 0.0f || r >= h)
    //     return 0.0f;

    float volume = (std::numbers::pi * std::pow(h, 8)) / 4.0f;
    float value = std::fmax(0.0f, std::pow(h, 2) - std::pow(r, 2));

    return std::pow(value, 3) / volume;
}

glm::vec2 Fluid::SmoothingKernelPoly6::calculateGradient(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r >= h)
        return glm::vec2(0, 0);

    float f = std::pow(h, 2) - std::pow(r, 2);
    float scale = -24 / (std::numbers::pi * std::pow(h, 8));
    return scale * r * static_cast<float>(std::pow(f, 2)) * distance->direction;
}

float Fluid::SmoothingKernelPoly6::calculateLaplacian(ParticleDistance *distance, float smoothingRadius)
{
    return 0.0f;
}