#include "App/ItemRegistrySetup.hpp"

namespace lve
{
    void ItemRegistrySetup::SetupItemRegistry(ItemRegistry &itemRegistry)
    {
        itemRegistry.Register({.itemId = 1, .itemName = "GrassyDirt", .maxStackSize = 64});
        itemRegistry.Register({.itemId = 2, .itemName = "Dirt", .maxStackSize = 64});
        itemRegistry.Register({.itemId = 3, .itemName = "Stone", .maxStackSize = 64});
        itemRegistry.Register({.itemId = 4, .itemName = "Vase", .maxStackSize = 64});
    }

}
