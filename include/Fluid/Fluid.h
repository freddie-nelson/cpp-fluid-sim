#pragma once

#include "./Particle.h"
#include "./AABB.h"

#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

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

        float smoothingRadius;
    };

    using Grid = std::unordered_map<std::pair<int, int>, std::vector<Particle *>, boost::hash<std::pair<int, int>>>;

    class Fluid
    {
    public:
        Fluid(FluidOptions &options);
        ~Fluid();

        void init();
        void update(float dt);

        std::vector<Particle *> &getParticles();
        void clearParticles();

        Grid &getGrid();

    private:
        void applyGravity(float dt);
        void applyVelocity(float dt);

        void applyBoundingBox();

        std::vector<Particle *> getParticlesOfInfluence(Particle *p);

        void updateGrid();
        void insertIntoGrid(Particle *p);
        std::pair<int, int> getGridKey(Particle *p);

        FluidOptions options;
        std::vector<Particle *> particles;

        Grid grid;
    };
}