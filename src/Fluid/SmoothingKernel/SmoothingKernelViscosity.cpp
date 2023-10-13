#include "../../../include/Fluid/SmoothingKernel/SmoothingKernelViscosity.h"

#include <math.h>
#include <numbers>

float Fluid::SmoothingKernelViscosity::calculate(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = (15 / (2 * std::numbers::pi * std::pow(h, 3)));
    float term1 = -(std::pow(r, 3) / (2 * std::pow(h, 3)));
    float term2 = std::pow(r, 2) / std::pow(h, 2);
    float term3 = h / (2 * r);

    return constant * (term1 + term2 + term3 - 1);
}

glm::vec2 Fluid::SmoothingKernelViscosity::calculateGradient(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return glm::vec2(0, 0);

    float constant = (15 / (2 * std::numbers::pi * std::pow(h, 3)));
    float term1 = -((3 * std::pow(r, 2)) / (2 * std::pow(h, 3)));
    float term2 = (2 * r) / std::pow(h, 2);
    float term3 = -(h / (2 * std::pow(r, 2)));

    float scaling = constant * (term1 + term2 + term3);

    return scaling * distance->direction;
}

float Fluid::SmoothingKernelViscosity::calculateLaplacian(ParticleDistance *distance, float smoothingRadius)
{
    float r = distance->distance;
    float h = smoothingRadius;

    if (r <= 0 || r >= h)
        return 0.0f;

    float constant = (45 / (std::numbers::pi * std::pow(h, 6)));

    return constant * (h - r);
}