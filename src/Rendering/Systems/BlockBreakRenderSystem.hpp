#pragma once
#include "Rendering/Core/lve_pipeline.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/IVec3Hash.h"
#include "Util/lve_frame_info.hpp"
#include "World/Blocks/Block.hpp"
#include "ECS/Components/InteractionProgress.hpp"

#include <memory>

namespace renderSystems
{
    class BlockBreakRenderSystem {
      public:
        BlockBreakRenderSystem(lve::LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~BlockBreakRenderSystem();

        BlockBreakRenderSystem(const BlockBreakRenderSystem &) = delete;
        BlockBreakRenderSystem &operator=(const BlockBreakRenderSystem &) = delete;

        // call every frame; hasHit=false skips drawing entirely
        void render(lve::FrameInfo &frameInfo, InteractionProgress &progress);

      private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        std::unique_ptr<lve::LveModel> createBlockBreakModel(lve::LveDevice &device, const lve::HighlightShape &shape, float inflate, std::string textureStage);

        lve::LveDevice &lveDevice;
        std::unique_ptr<lve::LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<lve::LveModel> cubeModel;

        void buildStageRegions();
        std::unique_ptr<lve::LveModel> createUnitCubeModel(lve::LveDevice &device); // replaces createBlockBreakModel
        std::array<glm::vec4, 4> stageRegions{};
    };

} // namespace renderSystems