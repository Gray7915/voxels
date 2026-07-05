#pragma once
#include <glm/glm.hpp>
#include <condition_variable>

#include "World/VoxelData.hpp"
#include "World/Chunk.hpp"

#include "Util/lve_util.hpp"

namespace lve
{
    struct GenJob
    {
        glm::ivec3 chunkCoord;
    };

    struct GenResult
    {
        glm::ivec3 chunkCoord;
        VoxelData data;
    };

    struct MeshJob
    {
        glm::ivec3 chunkCoord;
        glm::ivec3 worldOffset;
        std::unique_ptr<Chunk> chunk;
    };

    struct MeshResult
    {
        glm::ivec3 chunkCoord;
        std::vector<Vertex> verticies;
        std::vector<uint32_t> indices;
    };

    template <typename T>
    class ThreadSafeQueue
    {
    private:
        std::deque<T> queue;
        std::mutex mtx;
        std::condition_variable cv;

    public:
        void push(T item);
        bool try_pop(T &out);
        T wait_and_pop();
    };
}