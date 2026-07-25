#pragma once

#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Core/lve_renderer.hpp"
#include <vulkan/vulkan.h>

#include "Util/ray.hpp"
#include "Util/Types.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Camera.hpp"

#include "World/Area.hpp"
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
        void drawDebugWindow(float frameTime, glm::vec3 pos, const Transform &transform, const CameraComponent &camera, Area &area);
        void drawQuitMenu(float windowWidth, float windowHeight);
        void drawInv(InventoryComponent &component);
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
