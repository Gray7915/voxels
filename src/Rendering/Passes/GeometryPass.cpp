#include "GeometryPass.hpp"
#include <array>
#include <vulkan/vulkan.h>

namespace lve
{
    GeometryPass::GeometryPass(LveDevice &device, SwapChain &swapChain) : device{device}, swapChain{swapChain}
    {
        createRenderPass();
        createDepthResources();
        createFrameBuffers();
    }

    void GeometryPass::createRenderPass()
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = device.findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChain.getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void GeometryPass::createDepthResources()
    {
        VkFormat depthFormat = device.findDepthFormat();
        VkExtent2D swapChainExtent = swapChain.getSwapChainExtent();
        depthImages.resize(swapChain.imageCount());
        depthImageMemorys.resize(swapChain.imageCount());
        depthImageViews.resize(swapChain.imageCount());

        for (int i = 0; i < depthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            device.createImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImages[i],
                depthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }
    /*  void GeometryPass::createColorResources()
      {
          VkFormat colorFormat = swapChain.getSwapChainImageFormat();
          VkExtent2D extent = swapChain.getSwapChainExtent();

          colorImages.resize(swapChain.imageCount());
          colorImageMemorys.resize(swapChain.imageCount());
          colorImageViews.resize(swapChain.imageCount());

          for (size_t i = 0; i < swapChain.imageCount(); i++)
          {
              VkImageCreateInfo imageInfo{};
              imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
              imageInfo.imageType = VK_IMAGE_TYPE_2D;
              imageInfo.extent = {extent.width, extent.height, 1};
              imageInfo.mipLevels = 1;
              imageInfo.arrayLayers = 1;

              imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
              imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

              imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
              imageInfo.usage =
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_SAMPLED_BIT;

              imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
              imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

              device.createImageWithInfo(
                  imageInfo,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  colorImages[i],
                  colorImageMemorys[i]);

              VkImageViewCreateInfo viewInfo{};
              viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
              viewInfo.image = colorImages[i];
              viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
              viewInfo.format = imageInfo.format;

              viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              viewInfo.subresourceRange.levelCount = 1;
              viewInfo.subresourceRange.layerCount = 1;

              vkCreateImageView(device.device(), &viewInfo, nullptr, &colorImageViews[i]);
          }
      }
          */

    void GeometryPass::createFrameBuffers()
    {
        Framebuffers.resize(swapChain.imageCount());
        for (size_t i = 0; i < swapChain.imageCount(); i++)
        {
            std::array<VkImageView, 2> attachments = {swapChain.getImageView(i), depthImageViews[i]};

            VkExtent2D swapChainExtent = swapChain.getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(
                    device.device(),
                    &framebufferInfo,
                    nullptr,
                    &Framebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void GeometryPass::begin(VkCommandBuffer cmd, int frameIndex)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = Framebuffers[frameIndex];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};

        // color clear
        clearValues[0].color = {0.4f, 0.7f, 1.0f, 1.0f};

        // depth clear
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(
            cmd,
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);

        // Optional but recommended: move viewport/scissor here
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.getSwapChainExtent().width);
        viewport.height = static_cast<float>(swapChain.getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{{0, 0}, swapChain.getSwapChainExtent()};

        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void GeometryPass::end(VkCommandBuffer cmd)
    {
        vkCmdEndRenderPass(cmd);
    }
}
