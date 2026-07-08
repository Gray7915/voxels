#include "ImguiManager.hpp"
#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>

namespace lve
{
    ImguiManager::ImguiManager(LveDevice &lveDevice, LveWindow &lveWindow, LveRenderer &lveRenderer)
        : lveDevice{lveDevice}
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();

        initDescriptorPool();

        ImGui_ImplGlfw_InitForVulkan(lveWindow.getGLFWwindow(), true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = lveDevice.getInstance();
        init_info.PhysicalDevice = lveDevice.hardwareDevice();
        init_info.Device = lveDevice.device();
        auto indices = lveDevice.findPhysicalQueueFamilies();
        init_info.QueueFamily = indices.graphicsFamily;
        init_info.Queue = lveDevice.graphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = 2;
        init_info.ImageCount = lveRenderer.getSwapChain().imageCount();
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.RenderPass = lveRenderer.getUiRenderPass();
        ImGui_ImplVulkan_Init(&init_info);
    }

    ImguiManager::~ImguiManager()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(lveDevice.device(), descriptorPool, nullptr);
    }

    void ImguiManager::initDescriptorPool()
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize &pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(lveDevice.device(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create ImGui descriptor pool");
    }

    void ImguiManager::newFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiManager::render(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void ImguiManager::drawDebugWindow(float frameTime, glm::vec3 pos)
    {
        fpsAccumulator += frameTime;
        fpsFrameCount++;

        displayedFps = fpsFrameCount / fpsAccumulator;
        fpsAccumulator = 0.0f;
        fpsFrameCount = 0;
        ImGui::SetNextWindowSize(ImVec2(50, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(50, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2(100.f, 50.f));
        ImGui::Begin("Debug");
        // ImGui::Text("Hello Vulkan");
        ImGui::Text("FPS: %.1f", displayedFps);
        ImGui::Text("Frame time: %.3f ms", frameTime * 1000.0f);
        ImGui::Text("Player Position: x %.2f, y %.2f, z %.2f", pos.x, pos.y, pos.z);
        ImGui::End();
    }

    void ImguiManager::drawCrosshair(float windowWidth, float windowHeight)
    {
        ImDrawList *drawList = ImGui::GetForegroundDrawList();

        float centerX = windowWidth / 2.0f;
        float centerY = windowHeight / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;

        drawList->AddLine({centerX - crosshairSize - gapSize, centerY}, {centerX - gapSize, centerY}, ImColor(255, 255, 255), lineWidth);
        drawList->AddLine({centerX + gapSize, centerY}, {centerX + crosshairSize + gapSize, centerY}, ImColor(255, 255, 255), lineWidth);
        drawList->AddLine({centerX, centerY - crosshairSize - gapSize}, {centerX, centerY - gapSize}, ImColor(255, 255, 255), lineWidth);
        drawList->AddLine({centerX, centerY + gapSize}, {centerX, centerY + crosshairSize + gapSize}, ImColor(255, 255, 255), lineWidth);
        drawList->AddCircleFilled({centerX, centerY}, dotSize, ImColor(255, 255, 255));
    }

    void ImguiManager::drawQuitMenu(float windowWidth, float windowHeight)
    {
        ImGui::SetNextWindowSize(ImVec2(50, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(50, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2((windowWidth / 2) - 100, (windowHeight / 2) - 50));
        ImGui::Begin("new window");
        ImGui::Text("Hello Vulkan");
        ImGui::End();
    }

    void ImguiManager::drawInv(InventoryComponent &component)
    {
        ImGui::SetNextWindowSize(ImVec2(50, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(50, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2(100.f, 200.f));
        ImGui::Begin("Inventory");
        ImGui::Text("Slots: %zu", component.inventoryStacks.size());
        for (int i = 0; i < component.inventoryStacks.size(); i++)
        {
            auto &stack = component.inventoryStacks[i];

            if (!stack.has_value())
            {
                ImGui::Text("Empty");
                continue;
            }

            ImGui::Text("%s x%d", stack->getItem()->itemName.c_str(), stack->getStackCount());
        }
        ImGui::End();
    }

    void ImguiManager::activateMouse()
    {
    }
}
