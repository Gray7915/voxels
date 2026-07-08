#pragma once
#include <memory>
#include "Inventory/Item.hpp"
namespace lve
{
    class ItemStack
    {
    public:
        ItemStack(const Item *item) : item(item), durability(item->maxDurability), StackCount(1) {}

        ItemStack(const Item *item, int stackAmount) : item(item), durability(item->maxDurability), StackCount(stackAmount) {}

        ~ItemStack() = default;

        int getStackCount() { return StackCount; }
        void setStackCount(int newCount) { StackCount = newCount; }

        int getDurability() { return durability; }
        void setDurability(int newDurability) { durability = newDurability; }

        const Item *getItem() const { return item; }

    private:
        const Item *item;
        int StackCount = 0;
        int durability = 0;
    };
}
