#pragma once
#include "Rendering/Core/SwapChain.hpp"
#include "Rendering/Core/lve_device.hpp"

namespace lve
{
    enum class Mode { Overlay, Standalone };

    class UIRenderPass {
      public:
        UIRenderPass(LveDevice &device, SwapChain &swapChain, Mode mode);
        VkRenderPass getRenderPass() { return renderPass; };

        void begin(VkCommandBuffer cmd, int frameIndex);
        void end(VkCommandBuffer cmd);

      private:
        LveDevice &device;
        SwapChain &swapChain;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkFramebuffer> Framebuffers;

        // std::vector<VkImage> colorImages;
        // std::vector<VkDeviceMemory> colorImageMemorys;
        // std::vector<VkImageView> colorImageViews;

        void createRenderPass();
        void createDepthResources();

        // void createColorResources();

        void createFrameBuffers();
        VkFormat findDepthFormat(LveDevice device);

        Mode mode;
    };
} // namespace lve
