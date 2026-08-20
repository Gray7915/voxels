#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/noise.hpp"
#include "World/VoxelData.hpp"

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
        std::vector<std::thread> workers;
        ThreadSafeQueue<GenJob> jobQueue;
        ThreadSafeQueue<GenResult> resultQueue;
        std::atomic<bool> running{true};

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
