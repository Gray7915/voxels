#include "Rendering/Systems/OffScreenRenderSystem.hpp"
#include "App/TextureAtlas.hpp"
#include "World/Blocks/BlockRegistry.hpp"

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

    std::unique_ptr<LveModel> OffScreenRenderSystem::buildBlockModel(BlockId blockId) {
        auto blockDef = BlockRegistry::Get().GetBlockByID(static_cast<int>(blockId));
        if (!blockDef)
            return nullptr;

        LveModel::Builder builder{};

        for (u8 face = 0; face < static_cast<u8>(Math::Direction::COUNT) - 1; ++face) {
            const uint32_t baseIndex = static_cast<uint32_t>(builder.vertices.size());

            for (int vert = 0; vert < 4; ++vert) {
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                Vertex vertex{};
                vertex.position = CUBE_VERTICES[cubeVertex] - glm::vec3(0.5f);
                vertex.normal = CUBE_NORMALS[face];
                vertex.color = {0.1f, 0.1f, 0.1f};
                vertex.ao = 0.9f;
                const std::string &texName = blockDef->faces.at(face);
                const auto &atlas = TextureAtlas::Get();
                const auto it = atlas.atlasRegions.find(texName);
                if (it != atlas.atlasRegions.end()) {
                    const auto &region = it->second;
                    const vec2 uv = CUBE_UVS[vert];
                    vertex.uv = {(region.x + uv.x * region.width) / static_cast<float>(atlas.atlasWidth), (region.y + uv.y * region.height) / static_cast<float>(atlas.atlasHeight)};
                } else {
                    vertex.uv = CUBE_UVS[vert];
                }
                builder.vertices.push_back(vertex);
            }

            for (size_t i : FACE_INDICES)
                builder.indices.push_back(baseIndex + static_cast<uint32_t>(i));
        }

        if (builder.vertices.size() < 3)
            return nullptr;
        return std::make_unique<LveModel>(lveDevice, builder);
    }

    void OffScreenRenderSystem::renderInvItems(FrameInfo &frameInfo, BlockId blockId) {
        if (modelCache.find(blockId) == modelCache.end()) {
            auto model = buildBlockModel(blockId);
            if (!model)
                return;
            modelCache[blockId] = std::move(model);
        }
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
        proj[1][1] *= -1;

        SimplePushConstantData push{};
        push.mvp = proj * view;

        std::cout << "vertex count: " << modelCache[blockId]->modelVerticies.size() << '\n';
        std::cout << "index count: " << modelCache[blockId]->modelIindices.size() << '\n';
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

        modelCache[blockId]->bind(frameInfo.commandBuffer);
        modelCache[blockId]->draw(frameInfo.commandBuffer);
    }
} // namespace lve