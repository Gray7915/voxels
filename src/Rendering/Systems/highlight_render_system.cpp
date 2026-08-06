// highlight_render_system.cpp
#include "highlight_render_system.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <stdexcept>
#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include "World/Blocks/BlockRegistry.hpp"

namespace lve
{

    struct HighlightPushConstantData {
        glm::mat4 modelMatrix{1.f};
    };

    HighlightRenderSystem::HighlightRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
        // cubeModel = createOutlineModel(lveDevice); // from step 1
    }

    HighlightRenderSystem::~HighlightRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void HighlightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create highlight pipeline layout!");
        }
    }

    void HighlightRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        // KEY DIFFERENCES from the default config:
        // pipelineConfig.inputAssemblyInfo.topology = VK_TOPOLOG;
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE; // don't occlude things behind it... (see note below)
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        // depthTestEnable stays VK_TRUE so terrain in front still occludes the highlight
        pipelineConfig.rasterizationInfo.lineWidth = 16.0f;

        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/highlight_shader.vert.spv", "shaders/highlight_shader.frag.spv", pipelineConfig);
    }

    std::unique_ptr<LveModel> HighlightRenderSystem::createOutlineModel(LveDevice &device, const glm::vec3 &size, vec3 &cameraDirection, HighlightShape highlghtShape, float thickness) {
        LveModel::Builder builder{};

        const glm::vec3 color{0.f, 0.f, 0.f};

        auto addLine = [&](glm::vec3 start, glm::vec3 end) {
            uint32_t base = static_cast<uint32_t>(builder.vertices.size());

            glm::vec3 color{0.f, 0.f, 0.f};
            glm::vec3 dir = glm::normalize(end - start);
            glm::vec3 helper = glm::vec3(0, 1, 0);

            if (abs(glm::dot(dir, helper)) > 0.99f) {
                helper = glm::vec3(1, 0, 0);
            }

            glm::vec3 side = glm::normalize(glm::cross(dir, helper));
            glm::vec3 up = glm::normalize(glm::cross(side, dir));

            float half = thickness * 0.5f;

            glm::vec3 offset1 = side * half;
            glm::vec3 offset2 = up * half;

            glm::vec3 p[8] = {start - offset1 - offset2, start + offset1 - offset2, start + offset1 + offset2, start - offset1 + offset2,

                              end - offset1 - offset2,   end + offset1 - offset2,   end + offset1 + offset2,   end - offset1 + offset2};

            for (auto &pos : p) {
                builder.vertices.push_back({pos, color});
            }

            static constexpr uint32_t indices[] = {0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7, 0, 4, 5, 5, 1, 0, 3, 2, 6, 6, 7, 3, 1, 5, 6, 6, 2, 1, 0, 3, 7, 7, 4, 0};

            for (uint32_t i : indices) {
                builder.indices.push_back(base + i);
            }
        };

        for (glm::ivec2 edge : highlghtShape.highlighBoxEdges) {
            glm::vec3 start = highlghtShape.highlightBoxVerticies[edge.x];
            glm::vec3 end = highlghtShape.highlightBoxVerticies[edge.y];

            addLine(start, end);
        }

        return std::make_unique<LveModel>(device, builder);
    }

    void HighlightRenderSystem::render(FrameInfo &frameInfo, int blockID, ivec3 blockPos, vec3 boxSize, vec3 &cameraDirection) {
        if (blockID == 0)
            return;
        cubeModel = createOutlineModel(lveDevice, boxSize, cameraDirection, BlockRegistry::Get().GetBlockByID(blockID)->highlightShape, 0.005);

        lvePipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        HighlightPushConstantData push{};
        float inflate = 0.0001f;
        glm::vec3 origin{blockPos.x, blockPos.y, blockPos.z};
        glm::vec3 size = glm::vec3(1.f + 2.f * inflate);
        push.modelMatrix = glm::translate(glm::mat4(1.f), origin) * glm::scale(glm::mat4(1.f), size);

        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(HighlightPushConstantData), &push);

        cubeModel->bind(frameInfo.commandBuffer);
        cubeModel->draw(frameInfo.commandBuffer);
    }

} // namespace lve
