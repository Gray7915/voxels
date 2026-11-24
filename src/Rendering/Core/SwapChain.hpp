#pragma once

#include "lve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>
// std lib headers
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace lve
{
    class SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(LveDevice &deviceRef, VkExtent2D windowExtent);
        SwapChain(LveDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previouse);

        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        VkImageView getImageView(int index) const { return swapChainImageViews[index]; }
        size_t imageCount() const { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
        uint32_t getCurrentFrame() const { return static_cast<uint32_t>(currentFrame); }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }
        float extentAspectRatio()
        {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        bool compareSwapFormats(const SwapChain &swapChain) const
        {
            std::cout << "compare";
            return swapChain.swapChainImageFormat == swapChainImageFormat;
        }

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    private:
        void Init();
        void createSwapChain();
        void createImageViews();
        void createSyncObjects();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        LveDevice &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::shared_ptr<SwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };
}
