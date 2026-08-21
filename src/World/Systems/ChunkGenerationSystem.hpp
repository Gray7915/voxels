#pragma once
#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/ChunkGenWorkerPool.hpp"
#include "World/Generation/noise.hpp"

namespace lve
{
    class Area;
    class ChunkGenerationSystem {
      public:
        ChunkGenerationSystem(Area &area);
        ~ChunkGenerationSystem();

        void update();
        void requestGeneration(glm::ivec3 coord);

      private:
        Area &area;
        ChunkGenWorkerPool genPool{4};
        int worldSeed;
        Octave groundNoise;
        Octave groundNoise2;
        Octave groundNoise3;
        Octave largeGroundNoise;
        Octave modifierNoise;
        Octave treeNoise;
        Octave caveNoise;
        Octave caveNoise2;

        static int hashSeed(uint64_t seed) {
            seed ^= seed >> 33;
            seed *= 0xff51afd7ed558ccdULL;
            seed ^= seed >> 33;
            seed *= 0xc4ceb9fe1a85ec53ULL;
            seed ^= seed >> 33;
            return (int)(seed & 0x7fffffff);
        }
    };
} // namespace lve
