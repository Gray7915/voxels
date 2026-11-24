#pragma once

#include <vulkan/vulkan.h>

namespace lve{
    struct FrameInfo{
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
    };
}
