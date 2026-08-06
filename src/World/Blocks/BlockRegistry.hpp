#pragma once

#include <array>
#include <string>
#include "World/Blocks/Block.hpp"
#include "Util/Types.hpp"

namespace lve
{
    class BlockRegistry
    {
    public:
        static constexpr size_t MAX_BLOCKS = 65536;

        static BlockRegistry &Get()
        {
            static BlockRegistry instance;
            return instance;
        }

        void Register(Block block)
        {
            blocksByID[block.id] = block;
            blocksByName[block.name] = block.id;
        }

        inline const Block *GetBlockByID(uint16_t id) const
        {
            return &blocksByID[id];
        }

        uint16_t GetBlockIDByName(const std::string &name) const
        {
            auto it = blocksByName.find(name);
            if (it == blocksByName.end())
                return 0;

            return it->second;
        }

    private:
        std::array<Block, MAX_BLOCKS> blocksByID{};
        std::unordered_map<std::string, u16> blocksByName;
    };
}
