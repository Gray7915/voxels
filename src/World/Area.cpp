#include "Area.hpp"
#include "../Rendering/Core/lve_device.hpp"
#include <iostream>
#include "Util/math.hpp"

namespace lve
{
    Area::~Area() = default;

    Area::Area(LveDevice &lveDevice, glm::vec3 offset)
    {
        for (int x = offset.x - MinMaxOffset; x <= offset.x + MinMaxOffset; x++)
        {
            for (int z = offset.z - MinMaxOffset; z <= offset.z + MinMaxOffset; z++)
            {
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                glm::ivec3 worldPos = chunkCoord * glm::ivec3(16, 1, 16);
                chunks.emplace(chunkCoord, std::make_unique<Chunk>(lveDevice, worldPos));
            }
        }
    }

    void Area::tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex)
    {
        glm::ivec3 c = glm::ivec3(center) / glm::ivec3(16, 32, 16);

        for (auto it = chunks.begin(); it != chunks.end();)
        {
            const glm::ivec3 &coord = it->first;
            if (coord.x < c.x - MinMaxOffset || coord.x > c.x + MinMaxOffset ||
                coord.z < c.z - MinMaxOffset || coord.z > c.z + MinMaxOffset)
            {
                auto chunkPtr = std::shared_ptr<Chunk>(std::move(it->second));
                // Intentionally empty body — capturing chunkPtr keeps the Chunk
                // (and its Vulkan buffers) alive until this deletion job runs,
                // deferring destruction until the GPU is confirmed done with it.
                lveDevice.queueDeletion([chunk = chunkPtr]() {}, currentFrameIndex);
                it = chunks.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (int x = c.x - MinMaxOffset; x <= c.x + MinMaxOffset; x++)
        {
            for (int z = c.z - MinMaxOffset; z <= c.z + MinMaxOffset; z++)
            {
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                getOrCreateChunk(chunkCoord, lveDevice);
            }
        }
    }

    bool Area::isBlockSolid(glm::vec3 worldBlockPos)
    {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk)
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        return chunk->blocks[arrayPos.x][arrayPos.y][arrayPos.z] != 0;
    }

    Chunk *Area::getChunk(glm::ivec3 coord)
    {
        auto it = chunks.find(coord);
        if (it == chunks.end())
            return nullptr;
        return it->second.get();
    }

    const Chunk *Area::getChunk(glm::ivec3 coord) const
    {
        auto it = chunks.find(coord);
        if (it == chunks.end())
            return nullptr;
        return it->second.get();
    }

    Chunk &Area::getOrCreateChunk(glm::ivec3 coord, LveDevice &lveDevice)
    {
        auto it = chunks.find(coord);
        if (it != chunks.end())
            return *it->second;

        glm::ivec3 worldPos = coord * glm::ivec3(16, 32, 16);
        auto [inserted, ok] = chunks.emplace(coord, std::make_unique<Chunk>(lveDevice, worldPos));
        return *inserted->second;
    }
}