#include "World/Generation/ChunkGenWorkerPool.hpp"

namespace lve
{
    Octave noise;

    ChunkGenWorkerPool::ChunkGenWorkerPool(size_t threadCount)
    {
        for (size_t i = 0; i < threadCount; i++)
            workers.emplace_back([this]
                                 { workerLoop(); });
    }

    ChunkGenWorkerPool::~ChunkGenWorkerPool()
    {
        running = false;
        jobQueue.shutdown();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }

    void ChunkGenWorkerPool::workerLoop()
    {
        while (running)
        {
            GenJob job;
            if (!jobQueue.wait_and_pop(job))
                break;
            VoxelData data = generateTerrain(job.chunkCoord);
            resultQueue.push({job.chunkCoord, std::move(data)});
        }
    }

    VoxelData ChunkGenWorkerPool::generateTerrain(glm::ivec3 chunkCoord)
    {
        VoxelData data{};
        data.allocate();
        glm::ivec2 worldOrigin = glm::ivec2(chunkCoord.x, chunkCoord.z) * VoxelData::WIDTH;
        
        for (int x = 0; x < VoxelData::WIDTH; x++)
        {
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                glm::vec2 worldPos = glm::vec2(x + worldOrigin.x, z + worldOrigin.y);

                float heightValue = noise.sample(worldPos * 0.009f);
                heightValue = (heightValue + 1.0f) * 0.5f;

                int surfaceHeight = (int)(heightValue * (VoxelData::HEIGHT - 1));
                int stoneHeight = surfaceHeight - 3;

                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    if (y > surfaceHeight)
                        data.set(x, y, z, 0);
                    else if (y == surfaceHeight)
                        data.set(x, y, z, 1);
                    else if (y < surfaceHeight && y >= stoneHeight)
                        data.set(x, y, z, 2);
                    else
                        data.set(x, y, z, 3);
                }
            }
        }

        return data;
    }
}
