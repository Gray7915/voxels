#include "InteractionSystem.hpp"
#include "../Components/Transform.hpp"
#include "../Components/Input.hpp"
#include "../Components/AABBComponent.hpp"
#include "../../Util/ray.hpp"
#include "../../World/Area.hpp"
#include "../../World/ChunkRenderer.hpp"
#include "../../lve_util.hpp"

#include <GLFW/glfw3.h>

namespace lve
{
    extern Coordinator coordinator;

    void InteractionSystem::Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice)
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
            RayHit rayHit = ray.detectBlockHit(4.0f);

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
                std::cout << "Ray Hit " << rayHit.hitPosition.x << " " << rayHit.hitPosition.y << " " << rayHit.hitPosition.z << '\n';
                // std::cout << "Ray Orgin " << viewerObject.transform.translation.x << " " << viewerObject.transform.translation.y << " " << viewerObject.transform.translation.z << '\n';
                // std::cout << "Ray Direction " << forward.x << " " << forward.y << " " << forward.z << '\n';
                glm::ivec3 blockCoord = WorldToChunkArray(rayHit.hitPosition);
                glm::ivec3 chunkPosition = WorldToChunkId(rayHit.hitPosition);
                Area::chunks.find(chunkPosition)->second->blocks[blockCoord.x][blockCoord.y][blockCoord.z] = 0;
                // std::cout << "array place found" << '\n';
                // gameObjects.find(chunkPos)->second.model.reset();
                Area::chunks.find(chunkPosition)->second->chunkModel = ChunkRenderer::mesh(Area::chunks.find(chunkPosition)->second->blocks, lveDevice, {0, 0, 0});

                // std::cout << "chunk changed" << '\n';

                vkDeviceWaitIdle(lveDevice.device());
            }

            static bool rightWasPressed = false;
            bool rightIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightWasPressed && !rightIsPressed)
                rightWasPressed = false;

            if (rightIsPressed && !rightWasPressed && rayHit.hitPosition != glm::ivec3(-1.0f))
            {
                rightWasPressed = true;
                std::cout << "Ray Hit " << rayHit.hitPosition.x << " " << rayHit.hitPosition.y << " " << rayHit.hitPosition.z << '\n';
                glm::ivec3 blockPos = rayHit.hitPosition + rayHit.hitDirection;

                glm::ivec3 blockCoord = WorldToChunkArray(blockPos);
                glm::ivec3 chunkPosition = WorldToChunkId(blockPos);
                bool blockPlaceOkay = CheckBlockPlacement(transform, aabb, blockPos);
                if (Area::chunks.find(chunkPosition)->second->blocks[blockCoord.x][blockCoord.y][blockCoord.z] != 1 && !blockPlaceOkay)
                    Area::chunks.find(chunkPosition)->second->blocks[blockCoord.x][blockCoord.y][blockCoord.z] = 1;

                Area::chunks.find(chunkPosition)->second->chunkModel = ChunkRenderer::mesh(Area::chunks.find(chunkPosition)->second->blocks, lveDevice, {0, 0, 0});

                // std::cout << "chunk changed" << '\n';

                vkDeviceWaitIdle(lveDevice.device());
            }
        }
    }

    bool InteractionSystem::CheckBlockPlacement(const Transform &transform, const AABBComponent &aabbComponent, glm::ivec3 position)
    {
        glm::vec3 minPos = transform.position - aabbComponent.halfExtents;
        glm::vec3 maxPos = transform.position + aabbComponent.halfExtents;

        glm::ivec3 minBlock = glm::floor(minPos);
        glm::ivec3 maxBlock = glm::floor(maxPos);

        for (int x = minBlock.x; x <= maxBlock.x; ++x)
            for (int y = minBlock.y; y <= maxBlock.y; ++y)
                for (int z = minBlock.z; z <= maxBlock.z; ++z)
                {
                    if (glm::ivec3(glm::floor(glm::vec3(x, y, z))) == position)
                    {
                        return true;
                    }
                }
        return false;
    }
}
