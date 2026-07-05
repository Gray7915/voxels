#include <vector>
#include <thread>
#include <atomic>

#include "Util/ThreadSafeQueue.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    class ChunkMeshWorkerPool
    {
        void workerPool()
        {
            while (running)
            {
                MeshJob job = jobQueue.wait_and_pop();
                resultQueue.push(generateMesh(job));
            }
        }

        MeshResult generateMesh(MeshJob &job)
        {
            MeshResult result{};
            for (int x = 0; x < VoxelData::WIDTH; x++)
            {
                for (int z = 0; z < VoxelData::DEPTH; z++)
                {
                    for (int y = 0; y < VoxelData::HEIGHT; y++)
                    {
                        emitBlock(job, result, glm::ivec3(x, y, z));
                    }
                }
            }
        }

        void emitBlock(MeshJob &job, MeshResult result, glm::ivec3 pos)
        {
            const auto uv_unit = glm::vec2(1.0f) / glm::vec2(16.0f);

            for (int face = 0; face < 6; face++)
            {
                glm::ivec3 n = pos + getDirection(face);
                bool visible = n.x < 0 || n.y < 0 || n.z < 0 || n.x >= 18 || n.y >= 128 || n.z >= 18 || job.chunk->voxelData.get(n.x, n.y, n.z) == 0 || job.chunk->voxelData.get(n.x, n.y, n.z) == 4;
            }
        }

        static glm::ivec3 getDirection(int i)
        {
            return ((glm::ivec3[]){
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0),
                glm::ivec3(0, -1, 0)})[i];
        }

    public:
        std::vector<std::thread> workers;
        ThreadSafeQueue<MeshJob> jobQueue;
        ThreadSafeQueue<MeshResult> resultQueue;
        std::atomic<bool> running{true};

        bool tryGetResult(MeshResult &out)
        {
            return resultQueue.try_pop(out);
        }
    };
}