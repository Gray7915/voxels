#include "lve_renderer.hpp"

#include <stdexcept>
#include <array>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstring>
namespace lve
{

    LveRenderer::LveRenderer(LveWindow &window, LveDevice &device) : lveWindow{window}, lveDevice{device}, accelStructure{device}
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    LveRenderer::~LveRenderer()
    {
        freeCommandBuffers();
    }

    void LveRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(lveDevice.device(),
                             lveDevice.getCommandPool(),
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());

        commandBuffers.clear();
    }

    void LveRenderer::recreateSwapChain()
    {
        auto extent = lveWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.device());
        if (swapChain == nullptr)
        {
            swapChain = std::make_unique<SwapChain>(lveDevice, extent);
        }
        else
        {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_unique<SwapChain>(lveDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*swapChain.get()))
            {
                throw std::runtime_error("swap chain image or depth format has changed");
            }
        }

        createGeometryPass();
        createShadowPass();
        createUIPass();
        createCompositePass();
    }

    void LveRenderer::createUIPass()
    {
        UiRenderPass = std::make_unique<UIRenderPass>(
            lveDevice,
            *swapChain);
    }

    void LveRenderer::createGeometryPass()
    {
<<<<<<< HEAD
        auto extent = swapChain->getSwapChainExtent();

        gBuffer = std::make_unique<GBuffer>(
            lveDevice,
            extent);

        geometryPass = std::make_unique<GeometryPass>(
            lveDevice,
            *gBuffer,
            extent);
=======
        geometryPass = std::make_unique<GeometryPass>(lveDevice, *gBuffer, swapChain->getSwapChainExtent());
>>>>>>> 6b374db (some stuff)
    }

    void LveRenderer::createShadowPass()
    {
        auto extent = swapChain->getSwapChainExtent();

        shadowPass = std::make_unique<ShadowPass>(
            lveDevice,
            *gBuffer,
            extent);
    }

    void LveRenderer::createCompositePass()
    {
        auto extent = swapChain->getSwapChainExtent();

        compositePass = std::make_unique<CompositePass>(lveDevice, *gBuffer, shadowPass->getShadowMaskView(), UiRenderPass->getRenderPass(), extent, *swapChain);
    }

    void LveRenderer::createCommandBuffers()
    {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    VkCommandBuffer LveRenderer::beginFrame()
    {
        assert(!isFrameStarted && "can't call begin frame when already in progress");

        auto result = swapChain->acquireNextImage(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image");
        }
        isFrameStarted = true;

        lveDevice.flushDeletionQueue(currentFrameIndex);

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffers");
        }
        return commandBuffer;
    }

    void LveRenderer::endFrame()
    {
        assert(isFrameStarted && "can't call end frame when frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to recrod command buffer!");
        }

        auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized())
        {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }
}
