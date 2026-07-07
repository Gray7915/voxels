#include "ECS/Systems/InventorySystem.hpp"
#include "ECS/Coordinator.hpp"
namespace lve
{
    extern Coordinator coordinator;

    void InventorySystem::Update(Area &area)
    {
        for (auto const &entity : mEntities)
        {
            auto &InvComp = coordinator.GetComponent<InventoryComponent>(entity);
            for (auto &e : coordinator.eventBus.blockBroken.read())
            {
                if (e.brokenBy == entity)
                {
                    Chunk *chunk = area.getChunk(e.chunkPos);
                    if (!chunk || !chunk->voxelData.isGenerated())
                        continue;
                    switch (chunk->voxelData.get(e.blockPos.x, e.blockPos.y, e.blockPos.z))
                    {
                    case 1:
                        InvComp.one += 1;
                        break;
                    case 2:
                        InvComp.two += 1;
                        break;
                    case 3:
                        InvComp.three += 1;
                        break;
                    }
                }
            }
        }
    }
}