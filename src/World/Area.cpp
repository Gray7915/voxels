#include "Area.hpp"
#include "Rendering/Core/lve_device.hpp"
#include <iostream>

#include "Physics/aabb.hpp"
#include "Util/math.hpp"

#include "World/Blocks/Block.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Systems/ChunkGenerationSystem.hpp"
#include "World/voxel.hpp"

namespace lve
{
    Area::~Area() = default;

    Area::Area(LveDevice &lveDevice, u64 seed) : device(lveDevice), chunkGenSystem(*this), chunkMeshSystem(*this, device), chunkMutationSystem() {}

    void Area::updateArea() {
        chunkGenSystem.update();
        chunkMutationSystem.Update(*this);
    }
    void Area::tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex) {
        ivec3 c = ivec3(center) / ivec3(16, 32, 16);
        chunkMeshSystem.Update(lveDevice, currentFrameIndex);

        for (auto it = chunks.begin(); it != chunks.end();) {
            const glm::ivec3 &coord = it->first;
            if (coord.x < c.x - MinMaxOffset || coord.x > c.x + MinMaxOffset || coord.z < c.z - MinMaxOffset || coord.z > c.z + MinMaxOffset) {
                auto chunkPtr = std::shared_ptr<Chunk>(std::move(it->second));
                lveDevice.queueDeletion([chunk = chunkPtr]() {
                }, currentFrameIndex);
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }

        for (int x = c.x - MinMaxOffset; x <= c.x + MinMaxOffset; x++) {
            for (int z = c.z - MinMaxOffset; z <= c.z + MinMaxOffset; z++) {
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                getOrCreateChunk(chunkCoord, lveDevice, chunkGenSystem);
            }
        }
    }

    void Area::markNeighborChunksDirty(ivec3 centerChunkPos) {
        for (glm::ivec3 dir : Math::HorizontalCardinal) {
            Chunk *neighbor = getChunk(centerChunkPos + dir);
            if (neighbor && neighbor->chunkState == ChunkState::Uploaded)
                neighbor->chunkState = ChunkState::Dirty;
        }
    }

    void Area::markChunkDity(ivec3 chunkPos) { getChunk(chunkPos)->chunkState == ChunkState::Dirty; }

    void Area::setBlockAtPos(ivec3 Pos, BlockId id) {
        ivec3 chunkId = glm::ivec3(WorldToChunkId(Pos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return;
        ivec3 arrayPos = WorldToChunkArray(Pos);
        chunk->voxelData.set(arrayPos.x, arrayPos.y, arrayPos.z, id);
    }

    bool Area::isBlockSolid(glm::vec3 worldBlockPos) {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
    }

    // Check for non block blocks (fences ect)
    bool Area::isBlockSolid(glm::vec3 worldBlockPos, glm::vec3 rayPos, glm::vec3 rayDirection) {
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

        if (voxel.renderType == RenderType::Block) {
            return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
        } else if (voxel.renderType != RenderType::Invisible) {
            return CollisionDetection::rayBoxIntersection(rayPos, rayDirection, worldBlockPos, voxel.highlightBoxSize);
        } else {
            return false;
        }
    }

    uint16_t Area::getBlockID(glm::vec3 worldBlockPos) {
        glm::ivec3 chunkId = glm::ivec3(WorldToChunkId(worldBlockPos));
        Chunk *chunk = getChunk(chunkId);
        if (!chunk || !chunk->voxelData.isGenerated())
            return false;

        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        return chunk->voxelData.get(arrayPos.x, arrayPos.y, arrayPos.z);
    }

    Chunk *Area::getChunk(glm::ivec3 coord) {
        auto it = chunks.find(coord);
        if (it == chunks.end())
            return nullptr;
        return it->second.get();
    }

    const Chunk *Area::getChunk(glm::ivec3 coord) const {
        auto it = chunks.find(coord);
        if (it == chunks.end())
            return nullptr;
        return it->second.get();
    }

    Chunk &Area::getOrCreateChunk(glm::ivec3 coord, LveDevice &lveDevice, ChunkGenerationSystem &chunkGenSystem) {
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
} // namespace lve
