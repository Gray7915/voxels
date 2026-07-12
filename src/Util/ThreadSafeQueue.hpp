#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

#include <glm/glm.hpp>

#include "Rendering/Core/lve_device.hpp"

#include "World/VoxelData.hpp"
#include "World/NeighborVoxelInfo.hpp"
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
        VoxelData voxelData;
        NeighborVoxelInfo neighborVoxelData;
        LveDevice *device = nullptr;
        bool isFirstMesh = false;
    };

    struct MeshResult
    {
        glm::ivec3 chunkCoord;
        std::vector<Vertex> verticies;
        std::vector<uint32_t> indices;
        std::unique_ptr<LveModel> model;
        bool isFirstMesh = false;
    };

    template <typename T>
    class ThreadSafeQueue
    {
    public:
        void push(T item)
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                queue.push_back(std::move(item));
            }
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

        // Returns false if the queue was shut down and is empty — signals the
        // calling worker thread to exit its loop instead of blocking forever.
        bool wait_and_pop(T &out)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]
                    { return !queue.empty() || shuttingDown; });
            if (queue.empty()) // only true if shuttingDown and nothing left
                return false;
            out = std::move(queue.front());
            queue.pop_front();
            return true;
        }

        void shutdown()
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                shuttingDown = true;
            }
            cv.notify_all(); // wake every blocked worker so they can check shuttingDown and exit
        }

        void printSize()
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Queue Size " << queue.size() << '\n';
        }

    private:
        std::deque<T> queue;
        std::mutex mtx;
        std::condition_variable cv;
        bool shuttingDown = false;
    };
}
