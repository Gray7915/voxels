#pragma once
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

        void Register(Item item) { items.try_emplace(item.itemId, item); }

        const Item *GetItem(int id)
        {
            auto it = items.find(id);
            if (it == items.end())
                return nullptr;

            return &it->second;
        }

    private:
        std::unordered_map<int, Item> items;
    };
}
