#pragma once

#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "IVec3Hash.h"
#include "lve_frame_info.hpp"
#include "World/Chunk.hpp"

// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace lve
{
    class ChunkRenderSystem
    {
    public:
        ChunkRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~ChunkRenderSystem();

        ChunkRenderSystem(const ChunkRenderSystem &) = delete;
        ChunkRenderSystem &operator=(const ChunkRenderSystem &) = delete;

        void renderChunks(FrameInfo &frameInfo, std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> &chunks);

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
