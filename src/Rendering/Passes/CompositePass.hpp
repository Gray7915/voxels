#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/GBuffer.hpp"
<<<<<<< HEAD
#include "Rendering/Core/SwapChain.hpp"
=======
>>>>>>> 6b374db (some stuff)

namespace lve
{
    struct CompositePushConstants
    {
<<<<<<< HEAD
        glm::vec4 sunDirection;
        glm::vec4 sunColor;
        glm::vec4 ambientColor;
=======
        glm::vec4 sunDirection; // xyz = direction, w = padding
        glm::vec4 sunColor;     // xyz = colour, w = intensity
        glm::vec4 ambientColor; // xyz = colour, w = padding
>>>>>>> 6b374db (some stuff)
    };

    class CompositePass
    {
    public:
<<<<<<< HEAD
        CompositePass(LveDevice &device, GBuffer &gbuffer, VkImageView shadowMaskView, VkRenderPass uiRenderPass, VkExtent2D extent, SwapChain &swapChain);

        ~CompositePass();

        VkRenderPass getRenderPass() const
        {
            return renderPass;
        }

        void begin(VkCommandBuffer cmd, VkFramebuffer framebuffer, VkExtent2D extent);

        void execute(VkCommandBuffer cmd, const CompositePushConstants &push);

        void end(VkCommandBuffer cmd);

    private:
        void createRenderPass();
=======
        CompositePass(LveDevice &device, GBuffer &gbuffer, VkImageView shadowMaskView, VkFormat swapchainFormat, VkExtent2D extent);
        ~CompositePass();

        void begin(VkCommandBuffer cmd, VkFramebuffer swapchainFramebuffer, VkExtent2D extent);
        void execute(VkCommandBuffer cmd, const CompositePushConstants &push);
        void end(VkCommandBuffer cmd);

        VkRenderPass getRenderPass() const { return renderPass; }

    private:
        void createRenderPass(VkFormat swapchainFormat);
>>>>>>> 6b374db (some stuff)
        void createDescriptors(VkImageView shadowMaskView);
        void createPipeline();

        LveDevice &device;
        GBuffer &gbuffer;
<<<<<<< HEAD
        SwapChain &swapChain;

        VkRenderPass renderPass{VK_NULL_HANDLE};

        VkPipeline pipeline{VK_NULL_HANDLE};
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

        VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
        VkDescriptorSetLayout descriptorLayout{VK_NULL_HANDLE};
        VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

        VkSampler sampler{VK_NULL_HANDLE};
=======

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
>>>>>>> 6b374db (some stuff)
    };
}
