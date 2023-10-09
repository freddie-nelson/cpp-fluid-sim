#pragma once

#include "./Particle.h"
#include "./AABB.h"

#include <vector>

namespace Fluid
{
    struct FluidOptions
    {
        int numParticles;
        float particleRadius;
        float particleSpacing;
        glm::vec2 initialCentre;

        glm::vec2 gravity;

        AABB boundingBox;
        float boudingBoxRestitution;
    };

    class Fluid
    {
    public:
        Fluid(FluidOptions &options);
        ~Fluid();

        void init();
        void update(float dt);

        void applyGravity(float dt);
        void applyVelocity(float dt);

        void applyBoundingBox();

        std::vector<Particle *> &getParticles();
        void clearParticles();

    private:
        FluidOptions options;
        std::vector<Particle *> particles;
    };
}