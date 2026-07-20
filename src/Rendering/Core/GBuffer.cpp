#include "GBuffer.hpp"
#include <stdexcept>
#include <array>

namespace lve
{
    GBuffer::GBuffer(LveDevice &device, VkExtent2D extent) : device{device}
    {
        depthFormat = device.findDepthFormat();
        createImages(extent);
        createDescriptors();
    }

    GBuffer::~GBuffer()
    {
        destroyImages();
        vkDestroyDescriptorSetLayout(device.device(), descriptorLayout, nullptr);
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
        vkDestroySampler(device.device(), sampler, nullptr);
    }

    void GBuffer::resize(VkExtent2D extent)
    {
        destroyImages();
        createImages(extent);
        // Re-update descriptors to point to new image views
        createDescriptors();
    }

    void GBuffer::createImages(VkExtent2D extent)
    {
        // Helper to create one image + view
        auto makeImage = [&](VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect,
                             VkImage &image, VkDeviceMemory &memory, VkImageView &view)

        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = format;
            imageInfo.extent = {extent.width, extent.height, 1};
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = usage;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       image, memory);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspect;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &view) != VK_SUCCESS)
                throw std::runtime_error("failed to create GBuffer image view!");
        };

        makeImage(VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_COLOR_BIT,
                  albedoImage, albedoMemory, albedoView);

        makeImage(VK_FORMAT_R16G16B16A16_SFLOAT,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_COLOR_BIT,
                  normalImage, normalMemory, normalView);

        makeImage(depthFormat,
                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_DEPTH_BIT,
                  depthImage, depthMemory, depthView);
    }

    void
    GBuffer::destroyImages()
    {
        vkDestroyImageView(device.device(), albedoView, nullptr);
        vkDestroyImageView(device.device(), normalView, nullptr);
        vkDestroyImageView(device.device(), depthView, nullptr);
        vkDestroyImage(device.device(), albedoImage, nullptr);
        vkDestroyImage(device.device(), normalImage, nullptr);
        vkDestroyImage(device.device(), depthImage, nullptr);
        vkFreeMemory(device.device(), albedoMemory, nullptr);
        vkFreeMemory(device.device(), normalMemory, nullptr);
        vkFreeMemory(device.device(), depthMemory, nullptr);
    }

    void GBuffer::createDescriptors()
    {
        // Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler);

        // Layout
        std::array<VkDescriptorSetLayoutBinding, 3> bindings{};
        for (int i = 0; i < 3; i++)
        {
            bindings[i].binding = i;
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT |
                                     VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorLayout);

        // Pool
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 3;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool);

        // Allocate
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorLayout;
        vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet);

        // Write
        std::array<VkDescriptorImageInfo, 3> imageInfos{};
        imageInfos[0] = {sampler, albedoView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        imageInfos[1] = {sampler, normalView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        imageInfos[2] = {sampler, depthView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

        std::array<VkWriteDescriptorSet, 3> writes{};
        for (int i = 0; i < 3; i++)
        {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = descriptorSet;
            writes[i].dstBinding = i;
            writes[i].descriptorCount = 1;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].pImageInfo = &imageInfos[i];
        }
        vkUpdateDescriptorSets(device.device(), 3, writes.data(), 0, nullptr);
    }
}
