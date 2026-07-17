#include "Area.hpp"
#include "Rendering/Core/lve_device.hpp"
#include <iostream>

#include "Util/math.hpp"
#include "Physics/aabb.hpp"

#include "World/Systems/ChunkGenerationSystem.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/Block.hpp"
#include "World/voxel.hpp"

namespace lve
{
    Area::~Area() = default;

    Area::Area(LveDevice &lveDevice, glm::vec3 offset, ChunkGenerationSystem &chunkGenSystem)
    {
        for (int x = offset.x - MinMaxOffset; x <= offset.x + MinMaxOffset; x++)
        {
            for (int z = offset.z - MinMaxOffset; z <= offset.z + MinMaxOffset; z++)
            {
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                // getOrCreateChunk(chunkCoord, lveDevice, chunkGenSystem);
            }
        }
    }

    void Area::tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex, ChunkGenerationSystem &chunkGenSystem)
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
                getOrCreateChunk(chunkCoord, lveDevice, chunkGenSystem);
            }
        }
    }

    bool Area::isBlockSolid(glm::vec3 worldBlockPos)
    {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        Voxel voxelData = chunk->voxelData.getVoxel(arrayPos.x, arrayPos.y, arrayPos.z);
        auto optionalVoxel = BlockRegistry::Get().GetBlockByID(voxelData.blockID);
        Block voxel;

        if (optionalVoxel)
            voxel = optionalVoxel->get();
        return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
    }

    bool Area::isBlockSolid(glm::vec3 worldBlockPos, glm::vec3 rayPos, glm::vec3 rayDirection)
    {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        Voxel voxelData = chunk->voxelData.getVoxel(arrayPos.x, arrayPos.y, arrayPos.z);
        auto optionalVoxel = BlockRegistry::Get().GetBlockByID(voxelData.blockID);
        Block voxel;

        if (optionalVoxel)
            voxel = optionalVoxel->get();

        if (voxel.renderType == RenderType::Block)
        {
            return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
        }
        else
        {
            return CollisionDetection::rayBoxIntersection(rayPos, rayDirection, worldBlockPos, voxel.highlightBoxSize);
        }
    }

    uint16_t Area::getBlockID(glm::vec3 worldBlockPos)
    {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
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

    Chunk &Area::getOrCreateChunk(glm::ivec3 coord, LveDevice &lveDevice, ChunkGenerationSystem &chunkGenSystem)
    {
        auto it = chunks.find(coord);
        if (it != chunks.end())
            return *it->second;

        glm::ivec3 worldPos = coord * glm::ivec3(16, 32, 16);
        auto [inserted, ok] = chunks.emplace(coord, std::make_unique<Chunk>(lveDevice, coord));
        inserted->second->chunkState = ChunkState::QueuedForGeneration;
        inserted->second->offset = worldPos;
        chunkGenSystem.requestGeneration(coord);
        return *inserted->second;
    }
}
