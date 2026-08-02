#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/noise.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    extern Octave noise;

    class ChunkGenWorkerPool {
      public:
        explicit ChunkGenWorkerPool(size_t threadCount = std::thread::hardware_concurrency());
        ~ChunkGenWorkerPool();

        void submit(GenJob job) { jobQueue.push(std::move(job)); }
        bool tryGetResult(GenResult &out) { return resultQueue.try_pop(out); }
        void workerLoop();

        VoxelData generateTerrain(GenJob job);

      private:
        std::vector<std::thread> workers;
        ThreadSafeQueue<GenJob> jobQueue;
        ThreadSafeQueue<GenResult> resultQueue;
        std::atomic<bool> running{true};
    };
} // namespace lve
