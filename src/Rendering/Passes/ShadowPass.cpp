#include "Rendering/Passes/ShadowPass.hpp"
#include <stdexcept>
#include <fstream>

namespace lve
{
    ShadowPass::ShadowPass(LveDevice &device, GBuffer &gbuffer, VkExtent2D extent)
        : device{device}, gbuffer{gbuffer}
    {
        createShadowMaskImage(extent);
        createDescriptorLayout();
        createPipeline();
    }

    ShadowPass::~ShadowPass()
    {
        vkDestroyPipeline(device.device(), pipeline, nullptr);
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device.device(), descriptorLayout, nullptr);
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
        vkDestroyImageView(device.device(), shadowMaskView, nullptr);
        vkDestroyImage(device.device(), shadowMaskImage, nullptr);
        vkFreeMemory(device.device(), shadowMaskMemory, nullptr);
    }

    void ShadowPass::createShadowMaskImage(VkExtent2D extent)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8_UNORM;
        imageInfo.extent = {extent.width, extent.height, 1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | // compute writes
                          VK_IMAGE_USAGE_SAMPLED_BIT;  // composite reads
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   shadowMaskImage, shadowMaskMemory);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = shadowMaskImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &shadowMaskView) != VK_SUCCESS)
            throw std::runtime_error("failed to create shadow mask image view!");
    }

    void ShadowPass::createDescriptorLayout()
    {
        // binding 0: TLAS
        // binding 1: depth (sampler, to reconstruct world pos)
        // binding 2: shadow mask output (storage image)
        std::array<VkDescriptorSetLayoutBinding, 3> bindings{};

        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorLayout);

        // Pool
        std::array<VkDescriptorPoolSize, 3> poolSizes{{{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
                                                       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}}};

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorLayout;
        vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet);
    }

    void ShadowPass::execute(VkCommandBuffer cmd, VkExtent2D extent, VkAccelerationStructureKHR tlas, const ShadowPushConstants &push, int currentFrame)
    {
        std::cout
            << "ShadowPass execute "
            << this
            << " pipeline "
            << pipeline
            << "\n";
        if (tlas == VK_NULL_HANDLE)
        {
            std::cout << "Skipping shadow pass: no TLAS\n";

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            barrier.image =
                shadowMaskImage;

            barrier.subresourceRange =
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1};

            barrier.srcAccessMask = 0;

            barrier.dstAccessMask =
                VK_ACCESS_SHADER_READ_BIT;
            std::cout << "SHADOW BARRIER 1\n";
            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);

            return;
        }

        if (pipeline == VK_NULL_HANDLE)
        {
            throw std::runtime_error(
                "Shadow pipeline missing");
        }
        // Update TLAS descriptor (may change each frame)
        VkWriteDescriptorSetAccelerationStructureKHR tlasWrite{};
        tlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlasWrite.accelerationStructureCount = 1;
        tlasWrite.pAccelerationStructures = &tlas;

        // Depth sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        VkSampler sampler;
        vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler);

        VkDescriptorImageInfo depthInfo{
            sampler,
            gbuffer.getDepthView(),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
        VkDescriptorImageInfo shadowInfo{VK_NULL_HANDLE, shadowMaskView,
                                         VK_IMAGE_LAYOUT_GENERAL};

        std::array<VkWriteDescriptorSet, 3> writes{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].pNext = &tlasWrite;
        writes[0].dstSet = descriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = descriptorSet;
        writes[1].dstBinding = 1;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].pImageInfo = &depthInfo;

        writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[2].dstSet = descriptorSet;
        writes[2].dstBinding = 2;
        writes[2].descriptorCount = 1;
        writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writes[2].pImageInfo = &shadowInfo;

        vkUpdateDescriptorSets(device.device(), 3, writes.data(), 0, nullptr);

        // Transition shadow mask to general for compute write
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.image = shadowMaskImage;
        barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        std::cout << "SHADOW BARRIER 2\n";
        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        // Dispatch compute
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                           0, sizeof(ShadowPushConstants), &push);

        // One thread per pixel, 8x8 workgroups
        uint32_t groupX = (extent.width + 7) / 8;
        uint32_t groupY = (extent.height + 7) / 8;
        vkCmdDispatch(cmd, groupX, groupY, 1);

        // Transition shadow mask to shader read for composite pass
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        std::cout << "SHADOW BARRIER 3\n";
        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        device.queueDeletion(
            [device = &device, sampler]()
            {
                vkDestroySampler(device->device(), sampler, nullptr);
            },
            currentFrame);
        // std::cout << "Destroying sampler " << std::hex << sampler << std::endl;
    }

    void ShadowPass::createPipeline()
    {
        std::cout << "Creating shadow pipeline\n";
        VkPushConstantRange pushRange{};
        pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushRange.size = sizeof(ShadowPushConstants);

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorLayout;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushRange;
        vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &pipelineLayout);

        // Load shadow.comp.spv
        std::ifstream file("shaders/shadow.comp.spv", std::ios::binary | std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error("failed to open shadow.comp.spv!");
        size_t fileSize = file.tellg();
        std::cout << "SPIR-V size: "
                  << fileSize
                  << "\n";
        std::vector<char> code(fileSize);
        file.seekg(0);
        file.read(code.data(), fileSize);

        VkShaderModuleCreateInfo moduleInfo{};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = code.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(
                device.device(),
                &moduleInfo,
                nullptr,
                &shaderModule) != VK_SUCCESS)
        {
            std::cout << "FAILED SHADER MODULE\n";
            throw std::runtime_error(
                "failed to create shadow shader module");
        }
        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineInfo.stage.module = shaderModule;
        pipelineInfo.stage.pName = "main";

        VkResult result = vkCreateComputePipelines(
            device.device(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &pipeline);
        vkDestroyShaderModule(device.device(), shaderModule, nullptr);

        std::cout << "Compute pipeline result: "
                  << result
                  << "\n";

        std::cout << "Pipeline handle: "
                  << pipeline
                  << "\n";
    }
}
