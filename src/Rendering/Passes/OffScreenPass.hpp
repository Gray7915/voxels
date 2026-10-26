// OffscreenPass.hpp
#pragma once
#include "Rendering/Core/lve_device.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include "Util/Types.hpp"

namespace lve
{

    struct BlockPreviewTarget {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkSampler sampler;
        VkFramebuffer framebuffer;
        VkDescriptorSet descriptorSet; // handed to RmlUi
    };

    class OffScreenPass {
      public:
        OffScreenPass(LveDevice &device, uint32_t width, uint32_t height);
        ~OffScreenPass();

        // call before your geometry pass each frame
        void begin(VkCommandBuffer cmd, int blockId);
        void end(VkCommandBuffer cmd);

        // returns true if this block already has a preview rendered
        bool hasPreview(int blockId) const { return targets.count(blockId) > 0; }

        // creates a target for a new block id — call when hotbar changes
        void createPreviewForBlock(BlockId blockId);

        // get the descriptor set to hand to RmlUi
        VkDescriptorSet getDescriptorSet(int blockId) {
            auto it = targets.find(blockId);
            return it == targets.end() ? VK_NULL_HANDLE : it->second.descriptorSet;
        }

        VkRenderPass getRenderPass() const { return renderPass; }

      private:
        void createRenderPass();
        void createDepthResources();
        BlockPreviewTarget createTarget();
        void destroyTarget(BlockPreviewTarget &target);

        LveDevice &device;
        uint32_t width, height;

        VkRenderPass renderPass{VK_NULL_HANDLE};

        // shared depth buffer — all previews are same size so one is enough
        VkImage depthImage{VK_NULL_HANDLE};
        VkDeviceMemory depthMemory{VK_NULL_HANDLE};
        VkImageView depthView{VK_NULL_HANDLE};

        VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
        VkDescriptorPool descriptorPool{VK_NULL_HANDLE};

        std::unordered_map<int, BlockPreviewTarget> targets;
    };

} // namespace lve