#pragma once

#include "lve_camera.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "IVec3Hash.h"
// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace lve
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, const LveCamera &camera);

    private:
        void createPipelineLayout();
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
