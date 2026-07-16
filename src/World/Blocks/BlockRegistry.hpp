#pragma once

#include <utility>

#include "World/Blocks/Block.hpp"

namespace lve
{
    class BlockRegistry
    {
    public:
        static BlockRegistry &Get()
        {
            static BlockRegistry instance;
            return instance;
        }

        void Register(Block block)
        {
            blocksByNumeric.try_emplace(block.id, std::move(block));
            blocksByString.try_emplace(block.name, block.id);
        }

        const Block *GetBlockByID(int id)
        {
            auto item = blocksByNumeric.find(id);
            if (item == blocksByNumeric.end())
                return nullptr;

            return &item->second;
        }

        const Block *GetBlockByName(std::string name)
        {
            auto BlockID = blocksByString.find(name);
            if (BlockID == blocksByString.end())
                return nullptr;
            return GetBlockByID(BlockID->second);
        }

    private:
        std::unordered_map<int, Block> blocksByNumeric;
        std::unordered_map<std::string, int> blocksByString;
    };
}
