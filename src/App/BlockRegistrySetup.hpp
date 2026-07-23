#pragma once
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/Block.hpp"
#include "Rendering/Core/lve_device.hpp"

namespace lve
{
    class BlockRegistrySetup
    {
    public:
        static void SetupBlockRegistry(BlockRegistry &blockRegistry, LveDevice &device);
    };
}
