#include "../../../include/Fluid/SmoothingKernel/SmoothingKernelSpiky.h"

#include <math.h>
#include <numbers>

float Fluid::SmoothingKernelSpiky::calculate(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = (15 / (std::numbers::pi * std::pow(h, 6)));

    return constant * std::pow(h - r, 3);
}

glm::vec2 Fluid::SmoothingKernelSpiky::calculateGradient(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return glm::vec2(0, 0);

    float constant = -(45 / (std::numbers::pi * std::pow(h, 6)));
    float scaling = constant * std::pow(h - r, 2);

    return scaling * distance->direction;
}

float Fluid::SmoothingKernelSpiky::calculateLaplacian(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = 90 / (std::numbers::pi * std::pow(h, 6));

    return constant * (h - r);
}