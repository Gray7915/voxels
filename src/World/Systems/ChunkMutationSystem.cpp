#include "ChunkMutationSystem.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "World/Area.hpp"
#include "Util/ChunkState.hpp"
#include "Util/math.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void ChunkMutationSystem::Update(Area &area)
    {
        // std::cout << "break block queue size " << coordinator.eventBus.blockBroken.queueLength() << '\n';
        for (auto &e : coordinator.eventBus.blockBroken.read())
        {
            Chunk *chunk = area.getChunk(e.chunkPos);
            // std::cout << "block break request before gen check" << '\n';

            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            chunk->voxelData.set(e.blockPos.x, e.blockPos.y, e.blockPos.z, 0);
            std::cout << "block break request coord " << e.blockPos.x << ", " << e.blockPos.y << ", " << e.blockPos.z << '\n';
            chunk->chunkState = ChunkState::Dirty;
            std::cout << chunk->voxelData.get(4, 5, 4) << '\n';
            // std::cout << "block break request read" << '\n';
        }

        for (auto &req : coordinator.eventBus.blockPlaceRequested.read())
        {
            glm::ivec3 blockCoord = WorldToChunkArray(req.blockPos);
            glm::ivec3 chunkPos = WorldToChunkId(req.blockPos);

            Chunk *chunk = area.getChunk(chunkPos);
            if (!chunk || !chunk->voxelData.isGenerated())
                continue;

            auto &aabb = coordinator.GetComponent<AABBComponent>(req.placedBy);
            auto &transform = coordinator.GetComponent<Transform>(req.placedBy);

            if (CollisionDetection::CheckBlockPlacement(transform, aabb, req.blockPos))
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
