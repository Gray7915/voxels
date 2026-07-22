#include "ECS/Systems/InventorySystem.hpp"
#include "ECS/Coordinator.hpp"

#include "Inventory/ItemStack.hpp"
#include "Inventory/ItemRegistry.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void InventorySystem::Update(Area &area)
    {
        for (auto const &entity : mEntities)
        {
            auto &invComp = coordinator.GetComponent<InventoryComponent>(entity);

            for (auto &e : coordinator.eventBus.blockBreakRequest.read())
            {
                if (e.brokenBy != entity)
                    continue;

                Chunk *chunk = area.getChunk(e.chunkPos);

                if (!chunk || !chunk->voxelData.isGenerated())
                    continue;

                int itemId = chunk->voxelData.get(e.blockPos.x, e.blockPos.y, e.blockPos.z);

                bool added = false;

                for (auto &stack : invComp.inventoryStacks)
                {
                    if (!stack.has_value())
                        continue;

                    if (stack->getItem()->itemId == itemId &&
                        stack->getStackCount() < stack->getItem()->maxStackSize)
                    {
                        stack->setStackCount(stack->getStackCount() + 1);
                        added = true;
                        break;
                    }
                }

                if (!added)
                {
                    for (auto &stack : invComp.inventoryStacks)
                    {
                        if (!stack.has_value())
                        {
                            stack = ItemStack(ItemRegistry::Get().GetItemByID(itemId), 1);

                            added = true;
                            break;
                        }
                    }
                }
                if (!added)
                {
                    // Drop item into world implement in a tad
                }
            }
        }
    }
}
