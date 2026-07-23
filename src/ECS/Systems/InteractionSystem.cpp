#include "InteractionSystem.hpp"

#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/InventoryComponent.hpp"

#include "World/Area.hpp"

#include "Util/ray.hpp"
#include "Util/lve_util.hpp"
#include "Util/Types.hpp"

#include "Physics/aabb.hpp"

#include <GLFW/glfw3.h>

namespace lve
{
    extern Coordinator coordinator;

    void InteractionSystem::Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice, Area &area)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &camera = coordinator.GetComponent<CameraComponent>(entity);
            auto &inventory = coordinator.GetComponent<InventoryComponent>(entity);
            auto &input = coordinator.GetComponent<InputComponent>(entity);

            static bool escapeWasPressed = false;
            bool escapeIsPressed = glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS;
            if (escapeWasPressed && !escapeIsPressed)
                escapeWasPressed = false;

            if (escapeIsPressed && !escapeWasPressed)
            {
                escapeWasPressed = true;
                lveWindow.setMouseActive();
            }

            vec3 rot = transform.rotation;
            vec3 forward = {cos(rot.x) * sin(rot.y), -sin(rot.x), cos(rot.x) * cos(rot.y)};
            vec3 rayDir = glm::normalize(forward);

            Ray ray((transform.position + camera.relativePosition), rayDir);
            RayHit rayHit = ray.detectBlockHit(4.0f, area);

            hoveredID = {rayHit.hitPosition, rayHit.blockID};

            static bool pWasPressed = false;
            bool pIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
            if (pWasPressed && !pIsPressed)
                pWasPressed = false;

            if (pIsPressed && !pWasPressed && rayHit.hitPosition != ivec3(-1.0f) && lveWindow.getMenuActive())
            {
                pWasPressed = true;
                ivec3 blockCoord = WorldToChunkArray(rayHit.hitPosition);
                ivec3 chunkPosition = WorldToChunkId(rayHit.hitPosition);
                coordinator.eventBus.blockBreakRequest.push({chunkPosition, blockCoord, entity});
            }

            static bool rightWasPressed = false;
            bool rightIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightWasPressed && !rightIsPressed)
                rightWasPressed = false;

            if (rightIsPressed && !rightWasPressed && rayHit.hitPosition != ivec3(-1.0f) && lveWindow.getMenuActive())
            {
                rightWasPressed = true;
                ivec3 blockPos = rayHit.hitPosition + rayHit.hitDirection;
                ivec3 blockCoord = WorldToChunkArray(blockPos);
                ivec3 chunkPosition = WorldToChunkId(blockPos);

                auto &stack = inventory.inventoryStacks.at(input.hotbarSlot);
                BlockId id = 0;
                if (stack)
                {
                    id = stack->getItem()->itemId;
                    stack->setStackCount(stack->getStackCount() - 1);
                    coordinator.eventBus.blockPlaceRequested.push({blockCoord, chunkPosition, 4, entity});
                    if (stack->getStackCount() == 0)
                    {
                        inventory.inventoryStacks[input.hotbarSlot].reset();
                    }
                }
            }
        }
    }
}
