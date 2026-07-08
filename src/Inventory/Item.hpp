#pragma once

#include <memory>
#include <string>

#include "Rendering/Core/lve_model.hpp"

namespace lve
{
    class Item
    {
    public:
        int itemId = 0;
        std::string itemName;
        int maxStackSize = 64;
        int maxDurability = 0; //if 0 never breaks (i may actually not want durability)

        std::shared_ptr<LveModel> itemModel;
    };
}
