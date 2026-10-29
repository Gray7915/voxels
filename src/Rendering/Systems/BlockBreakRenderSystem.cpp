#include "Rendering/Systems/BlockBreakRenderSystem.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <cassert>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include "App/TextureAtlas.hpp"
#include "World/Blocks/BlockRegistry.hpp"

namespace renderSystems
{
    static const size_t UNIQUE_INDICES[] = {0, 1, 2, 5};
    static const size_t FACE_INDICES[] = {0, 1, 2, 0, 2, 3};
    static const size_t CUBE_INDICES[] = {
        4, 7, 6, 4, 6, 5, // south (+z)
        3, 0, 1, 3, 1, 2, // north (-z)
        7, 3, 2, 7, 2, 6, // east  (+x)
        0, 4, 5, 0, 5, 1, // west  (-x)
        2, 1, 5, 2, 5, 6, // up    (+y)
        0, 3, 7, 0, 7, 4  // down  (-y)
    };
    static const vec3 CUBE_VERTICES[] = {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}};
    static const vec3 CUBE_NORMALS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
    static const vec2 CUBE_UVS[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    // Crack stages, lowest to highest. Add more names to get more stages.
    static const std::array<std::string, 4> kStageNames = {
        "breaktexture_stage1",
        "breaktexture_stage2",
        "breaktexture_stage3",
        "breaktexture_stage4",
    };
    static constexpr int kStageCount = static_cast<int>(kStageNames.size());

    // How much bigger than the block the overlay is (stops z-fighting together with depth bias).
    // Keep it smaller than half the highlight outline thickness (0.0025).
    static constexpr float kInflate = 0.0002f;

    struct BlockBreakPushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::vec4 crackRegion{0.f}; // xy = atlas offset, zw = atlas size (normalised)
    };

    BlockBreakRenderSystem::BlockBreakRenderSystem(lve::LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
        buildStageRegions();
        cubeModel = createUnitCubeModel(lveDevice); // built once, never reassigned
    }

    BlockBreakRenderSystem::~BlockBreakRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void BlockBreakRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(BlockBreakPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create block break pipeline layout!");
        }
    }

    void BlockBreakRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr);

        lve::PipelineConfigInfo pipelineConfig{};
        lve::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        // Overlay: depth tested against terrain, never writes depth
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        // Pull the overlay slightly toward the camera to stop z-fighting
        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;
        // pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 4.0f;
        // pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 4.0f;

        // result = 2 * crack * block  (white brightens, mid-grey leaves unchanged, dark darkens)
        auto &blend = pipelineConfig.colorBlendAttachment;
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;

        lvePipeline = std::make_unique<lve::LvePipeline>(lveDevice, "shaders/breakblock.vert.spv", "shaders/breakblock.frag.spv", pipelineConfig);
    }

    void BlockBreakRenderSystem::buildStageRegions() {
        const auto &atlas = lve::TextureAtlas::Get();

        for (size_t i = 0; i < kStageNames.size(); ++i) {
            const auto it = atlas.atlasRegions.find(kStageNames[i]);
            if (it == atlas.atlasRegions.end()) {
                std::cerr << "BlockBreakRenderSystem: missing atlas region " << kStageNames[i] << '\n';
                stageRegions[i] = glm::vec4(0.f, 0.f, 1.f, 1.f); // will look wrong, but won't crash
                continue;
            }

            const auto &r = it->second;
            const float aw = static_cast<float>(atlas.atlasWidth);
            const float ah = static_cast<float>(atlas.atlasHeight);
            stageRegions[i] = glm::vec4(r.x / aw, r.y / ah, r.width / aw, r.height / ah);
        }
    }

    std::unique_ptr<lve::LveModel> BlockBreakRenderSystem::createUnitCubeModel(lve::LveDevice &device) {
        lve::LveModel::Builder builder{};

        for (u32 face = 0; face < 6; ++face) {
            const u32 base = static_cast<u32>(builder.vertices.size());

            for (int vert = 0; vert < 4; ++vert) {
                const size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                lve::Vertex v{};
                v.position = CUBE_VERTICES[cubeVertex];
                v.normal = CUBE_NORMALS[face];
                v.color = {1.f, 1.f, 1.f};
                v.uv = CUBE_UVS[vert]; // raw 0..1, mapped to the crack stage in the fragment shader
                v.ao = 1.f;
                builder.vertices.push_back(v);
            }

            for (size_t i : FACE_INDICES)
                builder.indices.push_back(base + static_cast<u32>(i));
        }

        return std::make_unique<lve::LveModel>(device, builder);
    }

    void BlockBreakRenderSystem::render(lve::FrameInfo &frameInfo, InteractionProgress &progress) {
        if (progress.InteractionTargetList.empty())
            return;

        VkCommandBuffer cmd = frameInfo.commandBuffer;

        lvePipeline->bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        cubeModel->bind(cmd); // once, before the loop

        for (const auto &t : progress.InteractionTargetList) {
            if (t.percentBroken <= 0.0)
                continue;

            const auto *block = lve::BlockRegistry::Get().GetBlockByID(t.targetId);
            if (!block)
                continue;

            const int stage = std::clamp(static_cast<int>(t.percentBroken * kStageCount), 0, kStageCount - 1);

            // Box size comes from the block; centred on x/z, sitting on the bottom of the block.
            const vec3 boxSize = block->highlightBoxSize;
            const vec3 size = boxSize + vec3(2.f * kInflate);
            const vec3 corner = vec3(t.targetPosition) + vec3((1.f - boxSize.x) * 0.5f, 0.f, (1.f - boxSize.z) * 0.5f) - vec3(kInflate);

            BlockBreakPushConstantData push{};
            push.modelMatrix = glm::translate(glm::mat4(1.f), corner) * glm::scale(glm::mat4(1.f), size);
            push.crackRegion = stageRegions[stage];

            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BlockBreakPushConstantData), &push);
            cubeModel->draw(cmd);
        }
    }
} // namespace renderSystems