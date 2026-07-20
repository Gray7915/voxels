#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "SwapChain.hpp"
#include "Rendering/Passes/UIRenderPass.hpp"
#include "Rendering/Passes/GeometryPass.hpp"
#include "Rendering/Passes/CompositePass.hpp"
#include "Rendering/Passes/ShadowPass.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace lve
{
    class LveRenderer
    {
    public:
        LveRenderer(LveWindow &lveWindow, LveDevice &lveDevice);
        ~LveRenderer();

        LveRenderer(const LveRenderer &) = delete;
        LveRenderer &operator=(const LveRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const
        {
            return geometryPass->getRenderPass();
        }

        VkRenderPass getUiRenderPass() const
        {
            return UiRenderPass->getRenderPass();
        }

        uint32_t getImageIndex() const
        {
            return currentImageIndex;
        }

        float getAspectRatio() const { return swapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }
        SwapChain &getSwapChain() { return *swapChain; }

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        void createGeometryPass();
        void createShadowPass();
        void createUIPass();
        void createCompositePass();

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        std::unique_ptr<GeometryPass> geometryPass;
        std::unique_ptr<UIRenderPass> UiRenderPass;

        std::unique_ptr<GBuffer> gBuffer;
        std::unique_ptr<ShadowPass> shadowPass;
        std::unique_ptr<CompositePass> compositePass;
        AccelerationStructure accelStructure;

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();
        // void createTextureImage();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        LveWindow &lveWindow;
        LveDevice &lveDevice;
        std::unique_ptr<SwapChain> swapChain;
        std::unique_ptr<SwapChain> oldSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;
        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted = false;
    };
}
