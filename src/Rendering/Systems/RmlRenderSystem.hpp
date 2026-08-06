#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_pipeline.hpp"
#include "Rendering/Core/lve_descriptors.hpp"
#include "Rendering/Core/lve_Texture.hpp"
#include "Ui/RmlRenderInterface.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include "Util/Types.hpp"

namespace lve
{
    struct RmlPushConstants {
        vec2 translation;
        vec2 screenSize;
        int useTexture;
    };

    class RmlRenderSystem {
      public:
        RmlRenderSystem(LveDevice &device, VkRenderPass renderPass, RmlRenderInterface &renderInterface);
        ~RmlRenderSystem();

        void render(VkCommandBuffer commandBuffer);
        VkDescriptorSet createTextureDescriptor(VkImageView imageView, VkSampler sampler);
        void createDescriptorLayout();

      private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        RmlRenderInterface &renderInterface;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        std::unique_ptr<LveDescriptorPool> rmlPool;
        std::unique_ptr<LveDescriptorSetLayout> rmlTextureLayout;
        std::unique_ptr<lve::LveTexture> fallbackTexture;
    };

} // namespace lve