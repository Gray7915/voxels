#include "ChunkMutationSystem.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkState.hpp"
#include "Util/math.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void ChunkMutationSystem::Update(Area &area)
    {
        for (auto &e : coordinator.eventBus.blockBreakRequest.read())
        {
            Chunk *chunk = area.getChunk(e.chunkPos);

            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            chunk->voxelData.set(e.blockPos.x, e.blockPos.y, e.blockPos.z, 0);
            chunk->chunkState = ChunkState::Dirty;
        }

        for (auto &req : coordinator.eventBus.blockPlaceRequested.read())
        {
            glm::ivec3 blockCoord = req.blockPos;
            glm::ivec3 chunkPos = req.chunkPos;

            Chunk *chunk = area.getChunk(chunkPos);

            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            auto &aabb = coordinator.GetComponent<AABBComponent>(req.placedBy);
            auto &transform = coordinator.GetComponent<Transform>(req.placedBy);

            if (!CollisionDetection::CheckBlockPlacement(transform, aabb, req.blockPos))
            {
                if (chunk->voxelData.get(blockCoord.x, blockCoord.y, blockCoord.z) == 0)
                {
                    chunk->voxelData.set(blockCoord.x, blockCoord.y, blockCoord.z, req.blockType);
                    chunk->chunkState = ChunkState::Dirty;
                }
            }
        }
    }
}
