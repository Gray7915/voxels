#include "ChunkMutationSystem.hpp"
#include "ECS/Coordinator.hpp"
#include "World/Area.hpp"
#include "Util/math.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include <assert.h>

namespace lve
{
    extern Coordinator coordinator;

    void ChunkMutationSystem::Update(Area &area)
    {
        for (auto &e : coordinator.eventBus.blockBroken.read())
        {
            auto it = area.chunks.find(e.chunkPos);
            assert(it != area.chunks.end());
            assert(it->second);
            auto &chunk = *it->second;
            chunk.blocks[e.blockPos.x][e.blockPos.y][e.blockPos.z] = 0;
            chunk.dirty = true;
        }

        for (auto &req : coordinator.eventBus.blockPlaceRequested.read())
        {
            glm::ivec3 blockCoord = WorldToChunkArray(req.blockPos);
            glm::ivec3 chunkPos = WorldToChunkId(req.blockPos);
            auto &chunk = *area.chunks.find(chunkPos)->second;

            // placement validity check (moved from InteractionSystem) goes here,
            // using coordinator.GetComponent<AABBComponent>(req.placedBy)
            auto &aabb = coordinator.GetComponent<AABBComponent>(req.placedBy);
            auto &transform = coordinator.GetComponent<Transform>(req.placedBy);

            if (CollisionDetection::CheckBlockPlacement(transform, aabb, req.blockPos))
            {
                if (chunk.blocks[blockCoord.x][blockCoord.y][blockCoord.z] == 0 /* && valid */)
                {
                    chunk.blocks[blockCoord.x][blockCoord.y][blockCoord.z] = req.blockType;
                    chunk.dirty = true;
                }
            }
        }
    }

}
