#pragma once

#include <vector>

#include "Inventory/Item.hpp"
#include "Inventory/ItemStack.hpp"

#include <optional>

struct InventoryComponent
{
    int selectedStack = 0;

    int InventoryCapacity = 5;

    std::vector<std::optional<lve::ItemStack>> inventoryStacks = std::vector<std::optional<lve::ItemStack>>(InventoryCapacity);
};
