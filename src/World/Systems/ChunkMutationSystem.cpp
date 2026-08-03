#include "ChunkMutationSystem.hpp"
#include "World/Area.hpp"
#include "World/Chunk.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Coordinator.hpp"
#include "Physics/aabb.hpp"
#include "Util/math.hpp"
#include "World/Generation/ChunkState.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void ChunkMutationSystem::Update(Area &area) {
        for (auto &e : coordinator.eventBus.blockBreakRequest.read()) {
            Chunk *chunk = area.getChunk(e.chunkPos);

            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            chunk->voxelData.set(e.blockPos.x, e.blockPos.y, e.blockPos.z, 0);
            chunk->chunkState = ChunkState::Dirty;
            area.markNeighborChunksDirty(e.chunkPos);
        }

        for (auto &req : coordinator.eventBus.blockPlaceRequested.read()) {
            glm::ivec3 blockCoord = req.blockPos;
            glm::ivec3 chunkPos = req.chunkPos;

            Chunk *chunk = area.getChunk(chunkPos);

            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            auto &aabb = coordinator.GetComponent<AABBComponent>(req.placedBy);
            auto &transform = coordinator.GetComponent<Transform>(req.placedBy);

            if (!CollisionDetection::CheckBlockPlacement(transform, aabb, req.blockPos)) {
                if (chunk->voxelData.get(blockCoord.x, blockCoord.y, blockCoord.z) == 0) {
                    chunk->voxelData.set(blockCoord.x, blockCoord.y, blockCoord.z, req.blockType);
                    auto &inventory = coordinator.GetComponent<InventoryComponent>(req.placedBy);
                    auto &stack = inventory.inventoryStacks.at(req.inventoryPos);
                    if (stack) {
                        stack->setStackCount(stack->getStackCount() - 1);
                        if (stack->getStackCount() == 0) {
                            inventory.inventoryStacks[req.inventoryPos].reset();
                        }
                    }
                    chunk->chunkState = ChunkState::Dirty;
                    area.markNeighborChunksDirty(req.chunkPos);
                }
            }
        }
    }
} // namespace lve
