#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <glm/glm.hpp>
#include "Util/ThreadSafeQueue.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    class ChunkMeshWorkerPool
    {
    public:
        explicit ChunkMeshWorkerPool(LveDevice &device, size_t threadCount = std::thread::hardware_concurrency());

        ~ChunkMeshWorkerPool();

        void submit(MeshJob job) { jobQueue.push(std::move(job)); }
        bool tryGetResult(MeshResult &out) { return resultQueue.try_pop(out); }

        void printJobQueueSize()
        {
            jobQueue.printSize();
        }

        void printResultQueue()
        {
            resultQueue.printSize();
        }

    private:
        void workerLoop();
        MeshResult generateMesh(MeshJob &job);
        void emitBlock(MeshJob &job, MeshResult &result, glm::ivec3 pos);
        int calculateAO(glm::ivec3 pos, int face, int vertexIndex, MeshJob &job);
        static glm::ivec3 getDirection(int i);
        static glm::vec2 getAtlasUV(int face, glm::vec2 uv, int blockType);

        glm::ivec3 getFaceTangent1(int face);
        glm::ivec3 getFaceTangent2(int face);

        int getSign(glm::ivec3 tangent, glm::ivec3 vertex)
        {
            if (tangent.x != 0)
                return vertex.x ? 1 : -1;

            if (tangent.y != 0)
                return vertex.y ? 1 : -1;

            return vertex.z ? 1 : -1;
        }

        bool getNeighborData(MeshJob &job, glm::ivec3 chunkVoxel);
        static constexpr float aoValues[] = {0.25f, 0.5f, 0.75f, 1.f};
        std::vector<std::thread> workers;
        ThreadSafeQueue<MeshJob> jobQueue;
        ThreadSafeQueue<MeshResult> resultQueue;
        std::atomic<bool> running{true};
        LveDevice &device;
        VkCommandPool myPool;
    };
}
