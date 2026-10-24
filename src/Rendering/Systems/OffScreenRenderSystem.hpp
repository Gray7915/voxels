#pragma once

#include "Rendering/Core/lve_pipeline.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/IVec3Hash.h"
#include "Util/lve_frame_info.hpp"
#include "World/Chunk.hpp"
#include "ECS/Coordinator.hpp"
#include "Util/Types.hpp"
// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace lve
{
    class OffScreenRenderSystem {
      public:
        OffScreenRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~OffScreenRenderSystem();

        OffScreenRenderSystem(const OffScreenRenderSystem &) = delete;
        OffScreenRenderSystem &operator=(const OffScreenRenderSystem &) = delete;

        void renderInvItems(FrameInfo &frameInfo, BlockId blockId);
        void clearCache() { modelCache.clear(); }

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

        std::unordered_map<BlockId, std::unique_ptr<LveModel>> modelCache;

        std::unique_ptr<LveModel> buildBlockModel(BlockId blockId);
        glm::mat4 buildInvProjection(u32 width, u32 height);
        glm::mat4 buildInvView();
    };
} // namespace lve
