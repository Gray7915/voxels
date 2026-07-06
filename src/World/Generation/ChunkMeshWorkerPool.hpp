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
        explicit ChunkMeshWorkerPool(size_t threadCount = std::thread::hardware_concurrency());
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
        static glm::ivec3 getDirection(int i);
        static glm::vec2 getAtlasUV(int face, glm::vec2 uv, int blockType);

        std::vector<std::thread> workers;
        ThreadSafeQueue<MeshJob> jobQueue;
        ThreadSafeQueue<MeshResult> resultQueue;
        std::atomic<bool> running{true};
    };
}
