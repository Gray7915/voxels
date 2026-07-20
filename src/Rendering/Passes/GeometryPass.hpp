#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/GBuffer.hpp"

namespace lve
{
    class GeometryPass
    {
    public:
        GeometryPass(LveDevice &device, GBuffer &gbuffer, VkExtent2D extent);
        ~GeometryPass();

        void begin(VkCommandBuffer cmd, VkExtent2D extent);
        void end(VkCommandBuffer cmd);

        VkRenderPass getRenderPass() const { return renderPass; }

    private:
        void createRenderPass();
        void createFramebuffer(VkExtent2D extent);

        LveDevice &device;
        GBuffer &gbuffer;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
    };
}