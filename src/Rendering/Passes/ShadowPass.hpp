#pragma once
#include <vulkan/vulkan.h>
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/GBuffer.hpp"
#include "Rendering/Core/AccelerationStructure.hpp"

namespace lve
{
    struct ShadowPushConstants
    {
        glm::vec3 sunDirection;
        float pad;
        glm::mat4 invViewProj;
    };

    class ShadowPass
    {
    public:
        ShadowPass(LveDevice &device, GBuffer &gbuffer, VkExtent2D extent);
        ~ShadowPass();

<<<<<<< HEAD
        void execute(VkCommandBuffer cmd, VkExtent2D extent, VkAccelerationStructureKHR tlas, const ShadowPushConstants &push, int currentFrame);
=======
        void execute(VkCommandBuffer cmd, VkExtent2D extent,
                     VkAccelerationStructureKHR tlas,
                     const ShadowPushConstants &push);
>>>>>>> 6b374db (some stuff)

        VkImageView getShadowMaskView() const { return shadowMaskView; }

    private:
        void createShadowMaskImage(VkExtent2D extent);
        void createDescriptorLayout();
        void createDescriptorSet(VkAccelerationStructureKHR tlas);
        void createPipeline();

        LveDevice &device;
        GBuffer &gbuffer;

        // Output shadow mask — R8 single channel, 1.0 = lit, 0.0 = shadowed
        VkImage shadowMaskImage = VK_NULL_HANDLE;
        VkDeviceMemory shadowMaskMemory = VK_NULL_HANDLE;
        VkImageView shadowMaskView = VK_NULL_HANDLE;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
}
