#include "World/Generation/ChunkGenWorkerPool.hpp"

namespace lve
{
    Octave noise;

    void ChunkGenWorkerPool::workerLoop()
    {
        while (running)
        {
            GenJob job = jobQueue.wait_and_pop();
            VoxelData data = generateTerrain(job.chunkCoord);
            resultQueue.push({job.chunkCoord, std::move(data)});
        }
    }

    VoxelData ChunkGenWorkerPool::generateTerrain(glm::ivec3 chunkCoord)
    {
        VoxelData data{};
        data.allocate();

        for (int x = 0; x < VoxelData::WIDTH; x++)
        {
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                glm::vec2 worldPos = glm::vec2(x + chunkCoord.x,
                                               z + chunkCoord.z);

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