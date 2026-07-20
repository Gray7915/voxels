#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{

    struct PipelineConfigInfo
    {

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
        uint32_t colorAttachmentCount = 2;
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    };

    class LvePipeline
    {
    public:
        LvePipeline(LveDevice &device, const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

        ~LvePipeline();

        LvePipeline(const LvePipeline &) = delete;
        LvePipeline &operator=(const LvePipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static PipelineConfigInfo defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        LveDevice &lveDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}
