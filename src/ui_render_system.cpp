#include "ui_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
namespace lve
{

    struct UIPushConstantData
    {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    UiRenderSystem::UiRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
        createCrosshair();
    }

    UiRenderSystem::~UiRenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void UiRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(UIPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void UiRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "shaders/ui.vert.spv",
            "shaders/ui.frag.spv",
            pipelineConfig);
    }

    void UiRenderSystem::renderUI(FrameInfo &frameInfo)
    {
        lvePipeline->bind(frameInfo.commandBuffer);

        crosshairModel->bind(frameInfo.commandBuffer);
        crosshairModel->draw(frameInfo.commandBuffer);
    }

    void UiRenderSystem::createCrosshair()
    {
        LveModel::Builder builder{};

        Vertex v0{};
        v0.position = {-0.1f, -0.1f, 0.0f};
        v0.color = {1.f, 1.f, 1.f};

        Vertex v1{};
        v1.position = {0.1f, -0.1f, 0.0f};
        v1.color = {1.f, 1.f, 1.f};

        Vertex v2{};
        v2.position = {0.1f, 0.1f, 0.0f};
        v2.color = {1.f, 1.f, 1.f};

        Vertex v3{};
        v3.position = {-0.1f, 0.1f, 0.0f};
        v3.color = {1.f, 1.f, 1.f};

        builder.vertices = {
            v0, v1, v2, v3};

        builder.indices = {
            0, 1, 2,
            2, 3, 0};

        crosshairModel =
            std::make_unique<LveModel>(
                lveDevice,
                builder);
    }
}
