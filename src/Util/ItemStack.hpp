#pragma once
#include <memory>
#include "Util/Item.hpp"
namespace lve
{
    class ItemStack
    {
        std::shared_ptr<Item> item;
        int count = 1;
        int durability = 0;
    };
}
