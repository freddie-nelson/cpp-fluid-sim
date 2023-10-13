#include "../../../include/Fluid/SmoothingKernel/SmoothingKernelPoly6.h"

#include <math.h>
#include <numbers>

float Fluid::SmoothingKernelPoly6::calculate(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = 315 / (64 * std::numbers::pi * std::pow(h, 9));

    return constant * std::pow(std::pow(h, 2) - std::pow(r, 2), 3);
}

glm::vec2 Fluid::SmoothingKernelPoly6::calculateGradient(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return glm::vec2(0, 0);

    float constant = -(945 / (32 * std::numbers::pi * std::pow(h, 9)));
    float scaling = constant * (r * std::pow(std::pow(h, 2) - std::pow(r, 2), 2));

    return scaling * distance->direction;
}

float Fluid::SmoothingKernelPoly6::calculateLaplacian(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = -(945 / (32 * std::numbers::pi * std::pow(h, 9)));

    return constant * (5 * std::pow(r, 4) - 6 * std::pow(h, 2) * std::pow(r, 2) + std::pow(h, 4));
}