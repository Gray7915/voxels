#include "Rendering/Systems/OffScreenRenderSystem.hpp"
#include "App/TextureAtlas.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/ConnectedBlock.hpp"
#include "World/Blocks/StagedBlock.hpp"
#include "World/Blocks/CreateVoxel.hpp"
#include "World/Blocks/IconVoxelContext.hpp"

namespace lve
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

    struct SimplePushConstantData {
        glm::mat4 mvp;
    };

    OffScreenRenderSystem::OffScreenRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    OffScreenRenderSystem::~OffScreenRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void OffScreenRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        // No descriptor sets — shader doesn't use any
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        std::vector<VkDescriptorSetLayout> layouts{globalSetLayout};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void OffScreenRenderSystem::createPipeline(VkRenderPass renderPass) {

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipelineConfig.bindingDescriptions = {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
        pipelineConfig.attributeDescriptions = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
        };

        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/offScreenUi.vert.spv", "shaders/offScreenUi.frag.spv", pipelineConfig);
    }

    std::unique_ptr<LveModel> OffScreenRenderSystem::emitIcon(BlockId blockId) {
        LveModel::Builder builder{};
        Blocks::IconVoxelContext ctx(static_cast<int>(blockId));
        Blocks::MeshOutput out{builder.vertices, builder.indices};

        Blocks::emitVoxel(ctx, out, ivec3(0));

        if (builder.vertices.size() < 3)
            return nullptr;
        return std::make_unique<LveModel>(lveDevice, builder);
    }

    void OffScreenRenderSystem::renderInvItems(FrameInfo &frameInfo, BlockId blockId) {
        if (modelCache.find(blockId) == modelCache.end()) {
            auto model = emitIcon(blockId);
            if (!model)
                return;
            modelCache[blockId] = std::move(model);
        }

        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::orthoRH_ZO(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
        glm::mat4 model = glm::rotate(glm::mat4(1.f), glm::radians(-30.f), {1, 0, 0}) * glm::rotate(glm::mat4(1.f), glm::radians(45.f), {0, 1, 0});
        proj[1][1] *= -1;
        SimplePushConstantData push{};
        push.mvp = proj * view * model;

        std::cout << "vertex count: " << modelCache[blockId]->modelVerticies.size() << '\n';
        std::cout << "index count: " << modelCache[blockId]->modelIindices.size() << '\n';
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

        modelCache[blockId]->bind(frameInfo.commandBuffer);
        modelCache[blockId]->draw(frameInfo.commandBuffer);
    }
} // namespace lve