#pragma once

#include <utility>

#include "Inventory/Item.hpp"

namespace lve
{
    class ItemRegistry
    {
    public:
        static ItemRegistry &Get()
        {
            static ItemRegistry instance;
            return instance;
        }

        void Register(Item item)
        {
            itemsByNumeric.try_emplace(item.itemId, std::move(item));
            itemsByString.try_emplace(item.itemName, item.itemId);
        }

        const Item *GetItemByID(int id)
        {
            auto item = itemsByNumeric.find(id);
            if (item == itemsByNumeric.end())
                return nullptr;

            return &item->second;
        }

        const Item *GetItemByName(std::string name)
        {
            auto itemID = itemsByString.find(name);
            if (itemID == itemsByString.end())
                return nullptr;
            return GetItemByID(itemID->second);
        }

    private:
        std::unordered_map<int, Item> itemsByNumeric;
        std::unordered_map<std::string, int> itemsByString;
    };
}
