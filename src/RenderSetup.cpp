#include "RenderSetup.hpp"
#include "lve_device.hpp"
#include "SwapChain.hpp"

namespace lve
{
    RenderSetup setupRender(LveDevice &lveDevice)
    {
        RenderSetup setup;

        setup.texture = std::make_unique<LveTexture>(lveDevice, "/home/patrick/Documents/Projects/voxels/Textures/images.jpeg");

        setup.globalPool = LveDescriptorPool::Builder(lveDevice)
                               .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                               .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                               .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                               .build();

        setup.uboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < setup.uboBuffers.size(); i++)
        {
            setup.uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            setup.uboBuffers[i]->map();
        }

        setup.globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        setup.globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < setup.globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = setup.uboBuffers[i]->descriptorInfo();

            VkDescriptorImageInfo imageInfo{};
            imageInfo.sampler = setup.texture->getSampler();
            imageInfo.imageView = setup.texture->getImageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            LveDescriptorWriter(*setup.globalSetLayout, *setup.globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(setup.globalDescriptorSets[i]);
        }

        return setup;
    }
}
