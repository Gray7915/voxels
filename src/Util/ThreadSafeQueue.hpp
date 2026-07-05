#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

#include <glm/glm.hpp>

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
        void push(T item)
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push_back(std::move(item));
            cv.notify_one();
        }

        bool try_pop(T &out)
        {
            std::lock_guard<std::mutex> lock(mtx);

            if (queue.empty())
                return false;

            out = std::move(queue.front());
            queue.pop_front();
            return true;
        }

        T wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [this]
                    { return !queue.empty(); });

            T value = std::move(queue.front());
            queue.pop_front();
            return value;
        }
    };
}