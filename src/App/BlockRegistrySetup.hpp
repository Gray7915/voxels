#pragma once
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/Block.hpp"

namespace lve
{
    class BlockRegistrySetup
    {
    public:
        static void SetupBlockRegistry(BlockRegistry &blockRegistry);
    };
}
