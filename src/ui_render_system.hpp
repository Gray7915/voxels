#pragma once

#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "IVec3Hash.h"
#include "lve_frame_info.hpp"
#include "lve_util.hpp"

// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace lve
{
    class UiRenderSystem
    {
    public:
        UiRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~UiRenderSystem();

        UiRenderSystem(const UiRenderSystem &) = delete;
        UiRenderSystem &operator=(const UiRenderSystem &) = delete;

        void renderUI(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        void createDescriptorSetLayout();
        void createDescriptorSet();
        void createCrosshair();

        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        LveDevice &lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkDescriptorSetLayout textureSetLayout;
        std::unique_ptr<LveModel> crosshairModel;
    };
}
