#pragma once // ChunkStagingPool.hpp
#include <mutex>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_buffer.hpp"
#include "Util/lve_util.hpp"

using namespace lve;

#include "Util/Types.hpp"

class ChunkStagingPool
{
public:
    static constexpr VkDeviceSize POOL_SIZE = 32 * 1024 * 1024; // 32MB
    static constexpr int MAX_SLOTS = 16;

    struct Slot
    {
        ivec3 chunkCoord;
        VkDeviceSize vertexOffset;
        VkDeviceSize vertexSize;
        VkDeviceSize indexOffset;
        VkDeviceSize indexSize;
        std::unique_ptr<LveBuffer> vertexDst;
        std::unique_ptr<LveBuffer> indexDst;
    };

    ChunkStagingPool(LveDevice &device)
    {
        stagingBuffer = std::make_unique<LveBuffer>(
            device,
            POOL_SIZE, 1, // treat as one big block
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer->map();
    }

    // Called from worker thread — claims a region and memcpys into it
    // Returns false if pool is full
    bool writeChunk(ivec3 coord, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, LveDevice &device)
    {
        VkDeviceSize vSize = sizeof(Vertex) * vertices.size();
        VkDeviceSize iSize = sizeof(uint32_t) * indices.size();

        std::lock_guard<std::mutex> lock(mutex);

        if (writeHead + vSize + iSize > POOL_SIZE)
            return false; // full this frame, retry next frame

        Slot slot{};
        slot.chunkCoord = coord;
        slot.vertexOffset = writeHead;
        slot.vertexSize = vSize;
        writeHead += vSize;
        slot.indexOffset = writeHead;
        slot.indexSize = iSize;
        writeHead += iSize;

        // memcpy into the persistently mapped staging buffer
        stagingBuffer->writeToBuffer((void *)vertices.data(), vSize, slot.vertexOffset);
        stagingBuffer->writeToBuffer((void *)indices.data(), iSize, slot.indexOffset);

        // pre-allocate device-local buffers (fast — just vkCreateBuffer + vkAllocateMemory)
        slot.vertexDst = std::make_unique<LveBuffer>(device,
                                                     sizeof(Vertex), (uint32_t)vertices.size(),
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        slot.indexDst = std::make_unique<LveBuffer>(device,
                                                    sizeof(uint32_t), (uint32_t)indices.size(),
                                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        pendingSlots.push_back(std::move(slot));
        return true;
    }

    // Called from main thread — records all pending copies into cmd, returns slots for model creation
    std::vector<Slot> recordCopies(VkCommandBuffer cmd)
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto &slot : pendingSlots)
        {
            VkBufferCopy vc{slot.vertexOffset, 0, slot.vertexSize};
            vkCmdCopyBuffer(cmd, stagingBuffer->getBuffer(),
                            slot.vertexDst->getBuffer(), 1, &vc);

            VkBufferCopy ic{slot.indexOffset, 0, slot.indexSize};
            vkCmdCopyBuffer(cmd, stagingBuffer->getBuffer(),
                            slot.indexDst->getBuffer(), 1, &ic);
        }
        return std::move(pendingSlots);
    }

    // Called from main thread after fence signals — reset for next frame
    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex);
        transferInFlight = false;
        writeHead = 0;
    }

    bool isTransferInFlight() const
    {
        return transferInFlight;
    }

private:
    std::unique_ptr<LveBuffer> stagingBuffer;
    std::vector<Slot> pendingSlots;
    VkDeviceSize writeHead = 0;
    bool transferInFlight = false;
    std::mutex mutex;
};
