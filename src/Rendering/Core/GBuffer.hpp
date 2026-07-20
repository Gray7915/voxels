#pragma once
#include <vulkan/vulkan.h>
#include "Rendering/Core/lve_device.hpp"

namespace lve
{
    class GBuffer
    {
    public:
        GBuffer(LveDevice &device, VkExtent2D extent);
        ~GBuffer();

        // Called on window resize
        void resize(VkExtent2D extent);

        VkImageView getAlbedoView() const { return albedoView; }
        VkImageView getNormalView() const { return normalView; }
        VkImageView getDepthView() const { return depthView; }

        // Descriptor set exposing all G-buffer images to shadow + composite passes
        VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
        VkDescriptorSetLayout getDescriptorLayout() const { return descriptorLayout; }

        VkFormat getDepthFormat() const { return depthFormat; }

    private:
        void createImages(VkExtent2D extent);
        void destroyImages();
        void createDescriptors();

        LveDevice &device;

        // Albedo — RGBA8
        VkImage albedoImage = VK_NULL_HANDLE;
        VkDeviceMemory albedoMemory = VK_NULL_HANDLE;
        VkImageView albedoView = VK_NULL_HANDLE;

        // Normal — RGBA16F
        VkImage normalImage = VK_NULL_HANDLE;
        VkDeviceMemory normalMemory = VK_NULL_HANDLE;
        VkImageView normalView = VK_NULL_HANDLE;

        // Depth — D32F
        VkImage depthImage = VK_NULL_HANDLE;
        VkDeviceMemory depthMemory = VK_NULL_HANDLE;
        VkImageView depthView = VK_NULL_HANDLE;
        VkFormat depthFormat;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
    };
}