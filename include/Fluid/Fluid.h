#pragma once

#include "./Particle.h"
#include "./AABB.h"
#include "./SmoothingKernel/SmoothingKernelPoly6.h"
#include "./SmoothingKernel/SmoothingKernelSpiky.h"

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

        float pressureLimit;
        float smoothingRadius;
        float stiffness;
        float desiredRestDensity;
        float particleMass;
        float viscosity;
        float surfaceTension;
        float surfaceTensionThreshold;

        bool usePredictedPositions;
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

        float solveDensityAtPoint(const glm::vec2 &point);

    private:
        void solveDensityPressure(Particle *p);
        void solvePressureForce(Particle *p);
        void solvePressureNearForce(Particle *p);
        void solveViscosityForce(Particle *p);
        void solveTensionForce(Particle *p);

        void applyGravity(Particle *p, float dt);
        void applySPHForces(Particle *p, float dt);
        void applyAttractors(Particle *p, float dt);
        void applyVelocity(Particle *p, float dt);

        void applyBoundingBox(Particle *p);

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
    };
}