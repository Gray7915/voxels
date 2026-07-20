#include "GeometryPass.hpp"
#include <stdexcept>
#include <array>

namespace lve
{
    GeometryPass::GeometryPass(LveDevice &device, GBuffer &gbuffer, VkExtent2D extent)
        : device{device}, gbuffer{gbuffer}
    {
        createRenderPass();
        createFramebuffer(extent);
    }

    GeometryPass::~GeometryPass()
    {
        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
        vkDestroyRenderPass(device.device(), renderPass, nullptr);
    }

    void GeometryPass::createRenderPass()
    {
        // Albedo attachment
        VkAttachmentDescription albedo{};
        albedo.format = VK_FORMAT_R8G8B8A8_UNORM;
        albedo.samples = VK_SAMPLE_COUNT_1_BIT;
        albedo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        albedo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        albedo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        albedo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Normal attachment
        VkAttachmentDescription normal = albedo;
        normal.format = VK_FORMAT_R16G16B16A16_SFLOAT;

        // Depth attachment
        VkAttachmentDescription depth{};
        depth.format = gbuffer.getDepthFormat();
        depth.samples = VK_SAMPLE_COUNT_1_BIT;
        depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        std::array<VkAttachmentDescription, 3> attachments = {albedo, normal, depth};

        VkAttachmentReference albedoRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference normalRef{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef{2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentReference, 2> colorRefs = {albedoRef, normalRef};

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthRef;

        VkSubpassDependency dep{};

        dep.srcSubpass = 0;
        dep.dstSubpass = VK_SUBPASS_EXTERNAL;

        dep.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        dep.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

        dep.srcAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        dep.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT;
        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        rpInfo.pAttachments = attachments.data();
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;
        rpInfo.dependencyCount = 1;
        rpInfo.pDependencies = &dep;

        if (vkCreateRenderPass(device.device(), &rpInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create geometry render pass!");
    }

    void GeometryPass::createFramebuffer(VkExtent2D extent)
    {
        std::array<VkImageView, 3> views = {
            gbuffer.getAlbedoView(),
            gbuffer.getNormalView(),
            gbuffer.getDepthView()};

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(views.size());
        fbInfo.pAttachments = views.data();
        fbInfo.width = extent.width;
        fbInfo.height = extent.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &fbInfo, nullptr, &framebuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create geometry framebuffer!");
    }

    void GeometryPass::begin(VkCommandBuffer cmd, VkExtent2D extent)
    {
        std::array<VkClearValue, 3> clearValues{};

        clearValues[0].color = {0.0f, 1.0f, 0.0f, 1.0f}; // RED albedo test
        clearValues[1].color = {0.0f, 1.0f, 0.0f, 0.0f}; // normal
        clearValues[2].depthStencil = {1.0f, 0};         // depth

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = renderPass;
        beginInfo.framebuffer = framebuffer;
        beginInfo.renderArea = {{0, 0}, extent};
        beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        beginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{0, 0,
                            static_cast<float>(extent.width),
                            static_cast<float>(extent.height),
                            0.0f, 1.0f};
        VkRect2D scissor{{0, 0}, extent};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }
    void GeometryPass::end(VkCommandBuffer cmd)
    {
        vkCmdEndRenderPass(cmd);
    }
}
