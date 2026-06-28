#pragma once
#include "lve_device.hpp"
#include "SwapChain.hpp"

namespace lve
{
    class UIRenderPass
    {
    public:
        UIRenderPass(LveDevice &device, SwapChain &swapChain);
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
    };
}
