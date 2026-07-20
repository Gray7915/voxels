#pragma once

#include "lve_window.hpp"

// std lib headers
#include <string>
#include <vector>
#include <functional>
#include <deque>
#include <mutex>

namespace lve
{

  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct QueueFamilyIndices
  {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
  };

  class LveDevice
  {
  public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    LveDevice(LveWindow &window);
    ~LveDevice();

    // Not copyable or movable
    LveDevice(const LveDevice &) = delete;
    LveDevice &operator=(const LveDevice &) = delete;
    LveDevice(LveDevice &&) = delete;
    LveDevice &operator=(LveDevice &&) = delete;

    VkCommandPool getCommandPool() { return commandPool; }
    VkDevice device() { return device_; }
    VkPhysicalDevice hardwareDevice() { return physicalDevice; };
    VkSurfaceKHR surface() { return surface_; }
    VkQueue graphicsQueue() { return graphicsQueue_; }
    VkQueue presentQueue() { return presentQueue_; }
    VkInstance getInstance() { return instance; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkFormat findDepthFormat() const;

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);
    VkCommandBuffer beginSingleTimeCommands() { return beginSingleTimeCommands(commandPool); } // main-thread default

    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) { endSingleTimeCommands(commandBuffer, commandPool); }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) { copyBuffer(srcBuffer, dstBuffer, size, commandPool); }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount, VkCommandPool pool);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
    {
      copyBufferToImage(buffer, image, width, height, layerCount, commandPool);
    }
    VkCommandPool createTransientCommandPool();
    void destroyCommandPool(VkCommandPool pool);

    void createImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);

    VkPhysicalDeviceProperties properties;
    VkImageView createImageView(VkImage image, VkFormat format);
    uint64_t getBufferDeviceAddress(VkBuffer buffer);

    void queueDeletion(std::function<void()> &&deleter, uint32_t frameIndex);
    void flushDeletionQueue(uint32_t currentFrame);
    std::mutex &getQueueMutex() { return queueMutex_; }

    float getTimestampPeriod() const
    {
      return properties.limits.timestampPeriod;
    }

    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructures;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;

  private:
    void
    createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    void printDeviceExtensions();
    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void loadRTFunctions();

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    LveWindow &window;

    VkCommandPool commandPool;

    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    };
    struct DeletionEntry
    {
      std::function<void()> deleter;
      uint32_t frameQueued;
    };

    std::deque<DeletionEntry> deletionQueue_;
    std::mutex deletionQueueMutex_;
    std::mutex queueMutex_;
    // static constexpr int MAX_FRAMES_IN_FLIGHT = SwapChain::MAX_FRAMES_IN_FLIGHT;
  };

} // namespace lve
