#pragma once

#include "./Particle.h"

#include <vector>

namespace Fluid
{
    struct FluidOptions
    {
        int numParticles;
        float particleRadius;
        float particleSpacing;
        glm::vec2 initialCentre;
    };

    class Fluid
    {
    public:
        Fluid(FluidOptions &options);
        ~Fluid();

        void init();
        void update(float dt);

        std::vector<Particle *> &getParticles();
        void clearParticles();

    private:
        FluidOptions options;
        std::vector<Particle *> particles;
    };
}