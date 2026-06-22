#pragma once

#include "lve_camera.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "IVec3Hash.h"
#include "lve_frame_info.hpp"

// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace lve
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, glm::ivec4 hoveredID);
        void renderChunk(VkCommandBuffer commandBuffer, std::vector<glm::ivec3>, const LveCamera &camera);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        void createDescriptorSetLayout();
        void createDescriptorSet();
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        LveDevice &lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkDescriptorSetLayout textureSetLayout;
    };
}
