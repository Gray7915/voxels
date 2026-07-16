#pragma once

#include <utility>
#include <iostream>
#include <optional>
#include <functional>
#include <unordered_map>
#include <string>

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
            blocksByNumeric.try_emplace(block.id, block);
            std::cout << " block id added " << block.id << '\n';
            blocksByString.try_emplace(block.name, block.id);
        }

        const std::optional<std::reference_wrapper<const Block>> GetBlockByID(uint16_t id)
        {
            auto item = blocksByNumeric.find(id);

            if (item == blocksByNumeric.end())
            {
                std::cout << "block id not found " << id << '\n';
                return std::nullopt;
            }

            std::cout << "gotten item id " << item->second.id << '\n';

            return item->second;
        }

        const std::optional<std::reference_wrapper<const Block>> GetBlockByName(std::string name)
        {
            auto BlockID = blocksByString.find(name);
            if (BlockID == blocksByString.end())
                return std::nullopt;
            return GetBlockByID(BlockID->second);
        }

    private:
        std::unordered_map<uint16_t, Block> blocksByNumeric;
        std::unordered_map<std::string, uint16_t> blocksByString;
    };
}
