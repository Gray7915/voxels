#include "RmlRenderSystem.hpp"
#include <RmlUi/Core/Vertex.h>
#include <stdexcept>

namespace lve
{

    RmlRenderSystem::RmlRenderSystem(LveDevice &device, VkRenderPass renderPass, RmlRenderInterface &renderInterface) : lveDevice(device), renderInterface(renderInterface) {
        createDescriptorLayout();
        createPipelineLayout();
        createPipeline(renderPass);
        renderInterface.setPipeline(lvePipeline->getPipeline(), pipelineLayout);
        renderInterface.setDescriptorLayout(rmlTextureLayout.get(), rmlPool.get(), this);
        uint8_t whitePixel[4] = {255, 255, 255, 255};
        fallbackTexture = std::make_unique<lve::LveTexture>(lveDevice, whitePixel, 1, 1);
        VkDescriptorSet fallbackSet = createTextureDescriptor(fallbackTexture->getImageView(), fallbackTexture->getSampler());
        renderInterface.setFallbackDescriptor(fallbackSet);
        assert(fallbackSet != VK_NULL_HANDLE);
    }

    RmlRenderSystem::~RmlRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void RmlRenderSystem::render(VkCommandBuffer commandBuffer) { renderInterface.beginFrame(commandBuffer); }

    void RmlRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(RmlPushConstants);

        std::vector<VkDescriptorSetLayout> layouts{rmlTextureLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create RmlUi pipeline layout");
    }

    void RmlRenderSystem::createDescriptorLayout() {
        // cppcheck-suppress missingReturn
        rmlTextureLayout = LveDescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();

        rmlPool = LveDescriptorPool::Builder(lveDevice).setMaxSets(64).addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64).build();
    }

    VkDescriptorSet RmlRenderSystem::createTextureDescriptor(VkImageView imageView, VkSampler sampler) {
        VkDescriptorSet descriptorSet;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler;
        imageInfo.imageView = imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        LveDescriptorWriter(*rmlTextureLayout, *rmlPool).writeImage(0, &imageInfo).build(descriptorSet);

        return descriptorSet;
    }

    void RmlRenderSystem::createPipeline(VkRenderPass renderPass) {
        PipelineConfigInfo config{};
        LvePipeline::defaultPipelineConfigInfo(config);
        config.renderPass = renderPass;
        config.pipelineLayout = pipelineLayout;

        config.depthStencilInfo.depthTestEnable = VK_FALSE;
        config.depthStencilInfo.depthWriteEnable = VK_FALSE;

        config.colorBlendAttachment.blendEnable = VK_TRUE;
        config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

        config.bindingDescriptions = {{0, sizeof(Rml::Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
        config.attributeDescriptions = {
            {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Rml::Vertex, position)},
            {1, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(Rml::Vertex, colour)},
            {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Rml::Vertex, tex_coord)},
        };

        lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/rml.vert.spv", "shaders/rml.frag.spv", config);
    }

} // namespace lve