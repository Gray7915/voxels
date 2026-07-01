#pragma once

#include "lve_device.hpp"
#include "lve_window.hpp"
#include "lve_renderer.hpp"
#include <vulkan/vulkan.h>

namespace lve
{
    class ImguiManager
    {
    public:
        ImguiManager(LveDevice &lveDevice, LveWindow &lveWindow, LveRenderer &lveRenderer);
        ~ImguiManager();

        ImguiManager(const ImguiManager &) = delete;
        ImguiManager &operator=(const ImguiManager &) = delete;

        void newFrame();
        void render(VkCommandBuffer commandBuffer);
        void drawCrosshair(float windowWidth, float windowHeight);
        void drawDebugWindow(float frameTime, glm::vec3 pos);
        void drawQuitMenu(float windowWidth, float windowHeight);
        void activateMouse();

    private:
        void initDescriptorPool();

        float fpsAccumulator = 0.0f;
        int fpsFrameCount = 0;
        float displayedFps = 0.0f;

        LveDevice &lveDevice;
        VkDescriptorPool descriptorPool;
    };
}
