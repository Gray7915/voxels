// highlight_render_system.cpp
#include "highlight_render_system.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <stdexcept>
#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

namespace lve
{

    struct HighlightPushConstantData
    {
        glm::mat4 modelMatrix{1.f};
    };

    HighlightRenderSystem::HighlightRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : lveDevice{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
        cubeModel = createWireframeCubeModel(lveDevice); // from step 1
    }

    HighlightRenderSystem::~HighlightRenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void HighlightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(HighlightPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create highlight pipeline layout!");
        }
    }

    void HighlightRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        // KEY DIFFERENCES from the default config:
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE; // don't occlude things behind it... (see note below)
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        // depthTestEnable stays VK_TRUE so terrain in front still occludes the highlight
        pipelineConfig.rasterizationInfo.lineWidth = 2.0f;

        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "shaders/highlight_shader.vert.spv",
            "shaders/highlight_shader.frag.spv",
            pipelineConfig);
    }

    void HighlightRenderSystem::render(FrameInfo &frameInfo, bool hasHit, glm::ivec3 blockPos, glm::vec3 direction)
    {
        if (!hasHit)
            return;
        std::cout << "Ray hit in render " <<direction.x << " " << direction.y << " " << direction.z << '\n';

        lvePipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1, &frameInfo.globalDescriptorSet,
            0, nullptr);

        HighlightPushConstantData push{};
        float inflate = 0.0001f;
        glm::vec3 origin = glm::vec3(blockPos) - glm::vec3(inflate);
        glm::vec3 size = glm::vec3(1.f + 2.f * inflate);
        push.modelMatrix = glm::translate(glm::mat4(1.f), origin) * glm::scale(glm::mat4(1.f), size);

        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(HighlightPushConstantData), &push);

        cubeModel->bind(frameInfo.commandBuffer);
        cubeModel->draw(frameInfo.commandBuffer);
    }

}
