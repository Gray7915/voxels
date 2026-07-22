// highlight_render_system.hpp
#pragma once
#include "Rendering/Core/lve_pipeline.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/IVec3Hash.h"
#include "Util/lve_frame_info.hpp"
#include <memory>

namespace lve
{

    class HighlightRenderSystem
    {
    public:
        HighlightRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~HighlightRenderSystem();

        HighlightRenderSystem(const HighlightRenderSystem &) = delete;
        HighlightRenderSystem &operator=(const HighlightRenderSystem &) = delete;

        // call every frame; hasHit=false skips drawing entirely
        void render(FrameInfo &frameInfo, bool hasHit, glm::ivec3 blockPos, glm::vec3 boxSize);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        std::unique_ptr<LveModel> createOutlineModel(LveDevice &device, const glm::vec3 &size, float thickness = 0.02f);

        LveDevice &lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<LveModel> cubeModel;
    };
}
