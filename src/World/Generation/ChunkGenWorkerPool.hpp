#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/noise.hpp"
#include "World/VoxelData.hpp"
#include "Util/Types.hpp"

namespace lve
{
    class ChunkGenWorkerPool {
      public:
        explicit ChunkGenWorkerPool(size_t threadCount = std::thread::hardware_concurrency());
        ~ChunkGenWorkerPool();

        void submit(GenJob job) { jobQueue.push(std::move(job)); }
        bool tryGetResult(GenResult &out) { return resultQueue.try_pop(out); }
        void workerLoop();

        VoxelData generateTerrain(GenJob job);

      private:
        float getNoiseValue(GenJob, ivec3 coord);
        bool isTreePosition(GenJob job, ivec3 coord);
        float getDensity(GenJob job, ivec3 worldCoord, float surfaceHeight);

        void runTerrainPass(GenJob job, VoxelData &data);
        void runCavePass(GenJob job, VoxelData &data);
        // void runOrePass(GenJob job, VoxelData &data);
        void runFloraPass(GenJob job, VoxelData &data);
        //  void runLootPass(GenJob job, VoxelData &data);

        void placeTree(VoxelData &data, int x, int y, int z);
        void setIfInBounds(VoxelData &data, int x, int y, int z, BlockId block);

        std::vector<std::thread> workers;
        ThreadSafeQueue<GenJob> jobQueue;
        ThreadSafeQueue<GenResult> resultQueue;
        std::atomic<bool> running{true};
    };
} // namespace lve
