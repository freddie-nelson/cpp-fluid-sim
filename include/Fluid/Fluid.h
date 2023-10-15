#pragma once

#include "./Particle.h"
#include "./AABB.h"
#include "./SmoothingKernel/SmoothingKernelPoly6.h"
#include "./SmoothingKernel/SmoothingKernelSpiky.h"
#include "./SmoothingKernel/SmoothingKernelViscosity.h"

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
        float stiffness;
        float desiredRestDensity;
        float particleMass;
        float viscosity;
        float surfaceTension;
        float surfaceTensionThreshold;
    };

    struct FluidAttractor
    {
        glm::vec2 position;
        float radius;
        float strength;
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

        void addAttractor(FluidAttractor *attractor);
        bool removeAttractor(FluidAttractor *attractor);
        void clearAttractors();

        Grid &getGrid();

    private:
        void applyGravity(float dt);
        void applySPHForces(float dt);
        void applyAttractors(float dt);
        void applyVelocity(float dt);

        void applyBoundingBox();

        void solveDensity();
        void solvePressure();
        void solvePressureForce();
        void solveViscosityForce();
        void solveTensionForce();

        std::vector<ParticleNeighbour> getParticlesOfInfluence(Particle *p, bool usePredictedPositions = false);

        void updateGrid(bool usePredictedPositions = false);
        void insertIntoGrid(Particle *p, bool usePredictedPositions = false);
        std::pair<int, int> getGridKey(Particle *p, bool usePredictedPositions = false);

        glm::vec2 randomDirection();

        FluidOptions options;
        std::vector<Particle *> particles;
        std::vector<FluidAttractor *> attractors;

        Grid grid;

        SmoothingKernelPoly6 smoothingKernelPoly6;
        SmoothingKernelSpiky smoothingKernelSpiky;
        SmoothingKernelViscosity smoothingKernelViscosity;
    };
}