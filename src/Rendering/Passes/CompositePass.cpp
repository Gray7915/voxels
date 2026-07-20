#include "CompositePass.hpp"
#include <array>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <assert.h>
namespace lve
{

    CompositePass::CompositePass(
        LveDevice &device,
        GBuffer &gbuffer,
        VkImageView shadowMaskView,
        VkRenderPass uiRenderPass,
        VkExtent2D extent)
        : device{device},
          gbuffer{gbuffer}
    {
        createRenderPass();
        createDescriptors(shadowMaskView);
        createPipeline();
    }

    CompositePass::~CompositePass()
    {
        vkDestroySampler(device.device(), sampler, nullptr);

        vkDestroyDescriptorPool(
            device.device(),
            descriptorPool,
            nullptr);

        vkDestroyDescriptorSetLayout(
            device.device(),
            descriptorLayout,
            nullptr);

        vkDestroyPipeline(
            device.device(),
            pipeline,
            nullptr);

        vkDestroyPipelineLayout(
            device.device(),
            pipelineLayout,
            nullptr);

        vkDestroyRenderPass(
            device.device(),
            renderPass,
            nullptr);
    }

    void CompositePass::createRenderPass()
    {
        VkAttachmentDescription color{};
        color.format = VK_FORMAT_B8G8R8A8_SRGB;
        color.samples = VK_SAMPLE_COUNT_1_BIT;

        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        color.initialLayout =
            VK_IMAGE_LAYOUT_UNDEFINED;

        color.finalLayout =
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint =
            VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};

        dependency.srcSubpass =
            VK_SUBPASS_EXTERNAL;

        dependency.dstSubpass = 0;

        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{};

        info.sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        info.attachmentCount = 1;
        info.pAttachments = &color;

        info.subpassCount = 1;
        info.pSubpasses = &subpass;

        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        if (vkCreateRenderPass(
                device.device(),
                &info,
                nullptr,
                &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed creating composite render pass");
        }
    }

    void CompositePass::createDescriptors(
        VkImageView shadowMaskView)
    {

        VkSamplerCreateInfo samplerInfo{};

        samplerInfo.sType =
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        samplerInfo.magFilter =
            VK_FILTER_LINEAR;

        samplerInfo.minFilter =
            VK_FILTER_LINEAR;

        samplerInfo.addressModeU =
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerInfo.addressModeV =
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        if (vkCreateSampler(
                device.device(),
                &samplerInfo,
                nullptr,
                &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed creating composite sampler");
        }

        std::array<VkDescriptorSetLayoutBinding, 4> bindings{};

        for (uint32_t i = 0; i < 4; i++)
        {
            bindings[i].binding = i;
            bindings[i].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            bindings[i].descriptorCount = 1;

            bindings[i].stageFlags =
                VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layout{};

        layout.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        layout.bindingCount =
            static_cast<uint32_t>(bindings.size());

        layout.pBindings =
            bindings.data();

        if (vkCreateDescriptorSetLayout(
                device.device(),
                &layout,
                nullptr,
                &descriptorLayout) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed creating composite descriptor layout");
        }

        VkDescriptorPoolSize pool{};

        pool.type =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        pool.descriptorCount = 4;

        VkDescriptorPoolCreateInfo poolInfo{};

        poolInfo.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

        poolInfo.maxSets = 1;

        poolInfo.poolSizeCount = 1;

        poolInfo.pPoolSizes = &pool;

        vkCreateDescriptorPool(
            device.device(),
            &poolInfo,
            nullptr,
            &descriptorPool);

        VkDescriptorSetAllocateInfo alloc{};

        alloc.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

        alloc.descriptorPool =
            descriptorPool;

        alloc.descriptorSetCount = 1;

        alloc.pSetLayouts = &descriptorLayout;

        vkAllocateDescriptorSets(
            device.device(),
            &alloc,
            &descriptorSet);

        std::array<VkDescriptorImageInfo, 4> images{};

        images[0] =
            {
                sampler,
                gbuffer.getAlbedoView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        images[1] =
            {
                sampler,
                gbuffer.getNormalView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        images[2] =
            {
                sampler,
                gbuffer.getDepthView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        images[3] =
            {
                sampler,
                shadowMaskView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        std::array<VkWriteDescriptorSet, 4> writes{};

        for (uint32_t i = 0; i < 4; i++)
        {
            writes[i].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            writes[i].dstSet =
                descriptorSet;

            writes[i].dstBinding = i;

            writes[i].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            writes[i].descriptorCount = 1;

            writes[i].pImageInfo = &images[i];
        }

        vkUpdateDescriptorSets(
            device.device(),
            4,
            writes.data(),
            0,
            nullptr);
    }

    void CompositePass::createPipeline()
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorLayout;

        if (vkCreatePipelineLayout(
                device.device(),
                &layoutInfo,
                nullptr,
                &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to create composite pipeline layout");
        }

        // ----------------------------
        // Load shaders
        // ----------------------------

        auto readFile = [](const std::string &filename)
        {
            std::ifstream file(
                filename,
                std::ios::ate | std::ios::binary);

            if (!file.is_open())
            {
                throw std::runtime_error(
                    "failed to open shader: " + filename);
            }

            size_t size =
                static_cast<size_t>(file.tellg());

            std::vector<char> buffer(size);

            file.seekg(0);

            file.read(
                buffer.data(),
                size);

            return buffer;
        };

        auto vertCode =
            readFile("shaders/composite.vert.spv");

        auto fragCode =
            readFile("shaders/composite.frag.spv");

        auto createShaderModule =
            [&](const std::vector<char> &code)
        {
            VkShaderModuleCreateInfo info{};

            info.sType =
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

            info.codeSize =
                code.size();

            info.pCode =
                reinterpret_cast<const uint32_t *>(
                    code.data());

            VkShaderModule module;

            if (vkCreateShaderModule(
                    device.device(),
                    &info,
                    nullptr,
                    &module) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "failed creating shader module");
            }

            return module;
        };

        VkShaderModule vertModule =
            createShaderModule(vertCode);

        VkShaderModule fragModule =
            createShaderModule(fragCode);

        VkPipelineShaderStageCreateInfo stages[2]{};

        stages[0].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        stages[0].stage =
            VK_SHADER_STAGE_VERTEX_BIT;

        stages[0].module =
            vertModule;

        stages[0].pName =
            "main";

        stages[1].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        stages[1].stage =
            VK_SHADER_STAGE_FRAGMENT_BIT;

        stages[1].module =
            fragModule;

        stages[1].pName =
            "main";

        // ----------------------------
        // Fullscreen triangle
        // no vertex buffers
        // ----------------------------

        VkPipelineVertexInputStateCreateInfo vertexInput{};

        vertexInput.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

        inputAssembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        inputAssembly.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        inputAssembly.primitiveRestartEnable =
            VK_FALSE;

        // ----------------------------
        // Dynamic viewport/scissor
        // ----------------------------

        VkPipelineViewportStateCreateInfo viewport{};

        viewport.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster{};

        raster.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        raster.depthClampEnable =
            VK_FALSE;

        raster.rasterizerDiscardEnable =
            VK_FALSE;

        raster.polygonMode =
            VK_POLYGON_MODE_FILL;

        raster.cullMode =
            VK_CULL_MODE_NONE;

        raster.frontFace =
            VK_FRONT_FACE_CLOCKWISE;

        raster.lineWidth =
            1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};

        multisample.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        multisample.rasterizationSamples =
            VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlend{};

        colorBlend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        colorBlend.blendEnable =
            VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorState{};

        colorState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        colorState.attachmentCount = 1;

        colorState.pAttachments =
            &colorBlend;

        VkPipelineDynamicStateCreateInfo dynamic{};

        std::array<VkDynamicState, 2> dynamics =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR};

        dynamic.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        dynamic.dynamicStateCount =
            static_cast<uint32_t>(
                dynamics.size());

        dynamic.pDynamicStates =
            dynamics.data();

        // ----------------------------
        // Create pipeline
        // ----------------------------

        VkGraphicsPipelineCreateInfo pipelineInfo{};

        pipelineInfo.sType =
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        pipelineInfo.stageCount = 2;

        pipelineInfo.pStages =
            stages;

        pipelineInfo.pVertexInputState =
            &vertexInput;

        pipelineInfo.pInputAssemblyState =
            &inputAssembly;

        pipelineInfo.pViewportState =
            &viewport;

        pipelineInfo.pRasterizationState =
            &raster;

        pipelineInfo.pMultisampleState =
            &multisample;

        pipelineInfo.pColorBlendState =
            &colorState;

        pipelineInfo.pDynamicState =
            &dynamic;

        pipelineInfo.layout =
            pipelineLayout;

        pipelineInfo.renderPass =
            renderPass;

        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(
                device.device(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed creating composite graphics pipeline");
        }

        vkDestroyShaderModule(
            device.device(),
            fragModule,
            nullptr);

        vkDestroyShaderModule(
            device.device(),
            vertModule,
            nullptr);

        std::cout
            << "Composite pipeline created: "
            << pipeline
            << "\n";
    }

    void CompositePass::begin(VkCommandBuffer cmd, VkFramebuffer framebuffer, VkExtent2D extent)
    {

        VkClearValue clear{};
        clear.color =
            {
                0.f,
                0.f,
                0.f,
                1.f};

        VkRenderPassBeginInfo begin{};

        begin.sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        begin.renderPass =
            renderPass;

        begin.framebuffer =
            framebuffer;

        begin.renderArea =
            {
                {0, 0},
                extent};

        begin.clearValueCount = 1;
        begin.pClearValues = &clear;

        vkCmdBeginRenderPass(
            cmd,
            &begin,
            VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};

        viewport.width =
            extent.width;

        viewport.height =
            extent.height;

        viewport.maxDepth = 1.f;

        VkRect2D scissor{};

        scissor.extent = extent;

        vkCmdSetViewport(
            cmd,
            0,
            1,
            &viewport);

        vkCmdSetScissor(
            cmd,
            0,
            1,
            &scissor);
    }

    void CompositePass::execute(
        VkCommandBuffer cmd)
    {

        vkCmdBindPipeline(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline);

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &descriptorSet,
            0,
            nullptr);

        // Fullscreen triangle
        vkCmdDraw(
            cmd,
            3,
            1,
            0,
            0);
    }

    void CompositePass::end(
        VkCommandBuffer cmd)
    {
        vkCmdEndRenderPass(cmd);
    }

}