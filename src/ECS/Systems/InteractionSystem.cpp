#include "InteractionSystem.hpp"

#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/AABBComponent.hpp"

#include "World/Area.hpp"
#include "World/ChunkRenderer.hpp"

#include "Util/ray.hpp"
#include "Util/lve_util.hpp"

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
            auto &aabb = coordinator.GetComponent<AABBComponent>(entity);
            auto &camera = coordinator.GetComponent<CameraComponent>(entity);

            glm::vec3 rot = transform.rotation;
            glm::vec3 forward = {cos(rot.x) * sin(rot.y), -sin(rot.x), cos(rot.x) * cos(rot.y)};
            glm::vec3 rayDir = glm::normalize(forward);

            Ray ray((transform.position + camera.relativePosition), rayDir);
            RayHit rayHit = ray.detectBlockHit(4.0f, area);

            if (rayHit.hitPosition == glm::ivec3(-1.0f))
            {
                hoveredID = glm::ivec4(-1.0f);
            }
            else
            {
                hoveredID = glm::ivec4(rayHit.hitPosition, 1);
            }

            static bool pWasPressed = false;
            bool pIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
            if (pWasPressed && !pIsPressed)
                pWasPressed = false;

            if (pIsPressed && !pWasPressed && rayHit.hitPosition != glm::ivec3(-1.0f))
            {
                pWasPressed = true;
                glm::ivec3 blockCoord = WorldToChunkArray(rayHit.hitPosition);
                glm::ivec3 chunkPosition = WorldToChunkId(rayHit.hitPosition);
                std::cout << "block place request made " << '\n';
                coordinator.eventBus.blockBreakRequest.push({chunkPosition, blockCoord, entity});
            }

            static bool rightWasPressed = false;
            bool rightIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightWasPressed && !rightIsPressed)
                rightWasPressed = false;

            if (rightIsPressed && !rightWasPressed && rayHit.hitPosition != glm::ivec3(-1.0f))
            {
                rightWasPressed = true;
                std::cout << "ray hit block: " << rayHit.hitPosition.x << " " << rayHit.hitPosition.y << " " << rayHit.hitPosition.z << '\n';
                glm::ivec3 blockPos = rayHit.hitPosition + rayHit.hitDirection;
                std::cout << "block place request corrd: " << blockPos.x << " " << blockPos.y << " " << blockPos.z << '\n';

                glm::ivec3 blockCoord = WorldToChunkArray(blockPos);
                glm::ivec3 chunkPosition = WorldToChunkId(blockPos);
                std::cout << "block place chunk cord in interaction system: " << chunkPosition.x << " " << chunkPosition.y << " " << chunkPosition.z << '\n';

                coordinator.eventBus.blockPlaceRequested.push({blockCoord, chunkPosition, 2, entity}); // hard code block to be placed for now
            }
        }
    }
}
