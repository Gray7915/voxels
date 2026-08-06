#include "World/Generation/ChunkGenWorkerPool.hpp"
#include "Util/Types.hpp"
namespace lve
{
    Octave noise;

    ChunkGenWorkerPool::ChunkGenWorkerPool(size_t threadCount) {
        for (size_t i = 0; i < threadCount; i++)
            workers.emplace_back([this] {
                workerLoop();
            });
    }

    ChunkGenWorkerPool::~ChunkGenWorkerPool() {
        running = false;
        jobQueue.shutdown();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }

    void ChunkGenWorkerPool::workerLoop() {
        while (running) {
            GenJob job;
            if (!jobQueue.wait_and_pop(job))
                break;
            VoxelData data = generateTerrain(job);
            resultQueue.push({job.chunkCoord, std::move(data)});
        }
    }

    VoxelData ChunkGenWorkerPool::generateTerrain(GenJob job) {
        VoxelData data{};
        data.allocate();
        ivec3 chunkCoord = job.chunkCoord;
        ivec2 worldOrigin = glm::ivec2(chunkCoord.x, chunkCoord.z) * VoxelData::WIDTH;

        for (int x = 0; x < VoxelData::WIDTH; x++) {
            for (int z = 0; z < VoxelData::DEPTH; z++) {
                vec2 worldPos = vec2(x + worldOrigin.x, z + worldOrigin.y);
                // std::cout << "seed before call to sample " << job.worldSeed << '\n';
                float heightValue = noise.sample(worldPos * 0.009f, job.worldSeed);
                heightValue = (heightValue + 1.0f) * 0.5f;

                int surfaceHeight = (int)(heightValue * (VoxelData::HEIGHT - 1));
                int stoneHeight = surfaceHeight - 3;
                // std::cout << "surface height " << surfaceHeight << '\n';
                for (int y = 0; y < VoxelData::HEIGHT; y++) {
                    if (y > surfaceHeight) {
                        data.set(x, y, z, 0);
                        // std::cout << "air set" << '\n';
                    } else if (y == surfaceHeight) {
                        data.set(x, y, z, 1);
                        // std::cout << "dirt set" << '\n';

                    } else if (y < surfaceHeight && y >= stoneHeight) {
                        data.set(x, y, z, 2);
                        // std::cout << "grass set" << '\n';

                    } else {
                        data.set(x, y, z, 3);
                        // std::cout << "stone set" << '\n';
                    }
                }
            }
        }
        return data;
    }
} // namespace lve