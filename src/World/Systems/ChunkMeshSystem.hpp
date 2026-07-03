#pragma once
#include "Rendering/Core/lve_device.hpp"

namespace lve
{
    class ChunkMeshSystem
    {
    public:
        void Update(LveDevice &device, uint32_t currentFrameIndex);
    };
}
