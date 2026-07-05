#pragma once

#include <vector>
#include <thread>
#include <atomic>

#include "Util/ThreadSafeQueue.hpp"
#include "Util/noise.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    extern Octave noise;

    class ChunkGenWorkerPool
    {
    public:
        std::vector<std::thread> workers;
        ThreadSafeQueue<GenJob> jobQueue;
        ThreadSafeQueue<GenResult> resultQueue;
        std::atomic<bool> running{true};

        void workerLoop();

        VoxelData generateTerrain(glm::ivec3 chunkCoord);
    };
}