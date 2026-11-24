#pragma once
#include "Inventory/ItemRegistry.hpp"
#include "Inventory/Item.hpp"

namespace lve
{
    class ItemRegistrySetup
    {
    public:
        static void SetupItemRegistry(ItemRegistry &itemRegistry);
    };
}
