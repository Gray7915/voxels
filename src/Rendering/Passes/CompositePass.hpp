#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/GBuffer.hpp"

namespace lve
{
    struct CompositePushConstants
    {
        alignas(16) glm::vec3 sunDirection;
        alignas(16) glm::vec3 sunColor;
        alignas(16) glm::vec3 ambientColor;
    };

    class CompositePass
    {
    public:
        CompositePass(
            LveDevice &device,
            GBuffer &gbuffer,
            VkImageView shadowMaskView,
            VkRenderPass uiRenderPass,
            VkExtent2D extent);

        ~CompositePass();

        VkRenderPass getRenderPass() const
        {
            return renderPass;
        }

        void begin(VkCommandBuffer cmd, VkFramebuffer framebuffer, VkExtent2D extent);

        void execute(VkCommandBuffer cmd);

        void end(VkCommandBuffer cmd);

    private:
        void createRenderPass();
        void createDescriptors(VkImageView shadowMaskView);
        void createPipeline();

        LveDevice &device;
        GBuffer &gbuffer;

        VkRenderPass renderPass{VK_NULL_HANDLE};

        VkPipeline pipeline{VK_NULL_HANDLE};
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

        VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
        VkDescriptorSetLayout descriptorLayout{VK_NULL_HANDLE};
        VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

        VkSampler sampler{VK_NULL_HANDLE};
    };
}