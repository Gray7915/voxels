#include "InteractionSystem.hpp"

#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/InteractionProgress.hpp"

#include "World/Area.hpp"

#include "Util/Types.hpp"
#include "Util/lve_util.hpp"
#include "Util/ray.hpp"

#include "Physics/aabb.hpp"

#include <GLFW/glfw3.h>
#include <chrono>
namespace lve
{
    void InteractionSystem::Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice, Area &area, Coordinator &coordinator) {
        for (auto const &entity : mEntities) {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &camera = coordinator.GetComponent<CameraComponent>(entity);
            auto &inventory = coordinator.GetComponent<InventoryComponent>(entity);
            auto &input = coordinator.GetComponent<InputComponent>(entity);
            auto &interactionProgress = coordinator.GetComponent<InteractionProgress>(entity);

            static bool escapeWasPressed = false;
            bool escapeIsPressed = glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS;
            if (escapeWasPressed && !escapeIsPressed)
                escapeWasPressed = false;

            if (escapeIsPressed && !escapeWasPressed) {
                escapeWasPressed = true;
                lveWindow.setMouseActive();
            }
            expireTargets(interactionProgress, deltaTime);
            vec3 rot = transform.rotation;
            vec3 forward = {cos(rot.x) * sin(rot.y), -sin(rot.x), cos(rot.x) * cos(rot.y)};
            vec3 rayDir = glm::normalize(forward);

            Ray ray((transform.position + camera.relativePosition), rayDir);
            RayHit rayHit = ray.detectBlockHit(4.0f, area);

            hoveredID = {rayHit.hitPosition, rayHit.blockID};

            static bool pWasPressed = false;
            bool pIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            if (pWasPressed && !pIsPressed)
                pWasPressed = false;

            if (pIsPressed && rayHit.hitPosition != ivec3(-1.0f) && lveWindow.getMenuActive()) {
                pWasPressed = true;
                updateMining(entity, deltaTime, area, coordinator, interactionProgress, rayHit);
            }

            static bool rightWasPressed = false;
            bool rightIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightWasPressed && !rightIsPressed)
                rightWasPressed = false;

            if (rightIsPressed && !rightWasPressed && rayHit.hitPosition != ivec3(-1.0f) && lveWindow.getMenuActive()) {
                rightWasPressed = true;
                ivec3 blockPos = rayHit.hitPosition + rayHit.hitDirection;
                ivec3 blockCoord = WorldToChunkArray(blockPos);
                ivec3 chunkPosition = WorldToChunkId(blockPos);
                auto &stack = inventory.inventoryStacks.at(input.hotbarSlot);
                BlockId id = 0;
                if (stack) {
                    id = stack->getItem()->itemId;
                    coordinator.eventBus.blockPlaceRequested.push({blockCoord, chunkPosition, id, entity, input.hotbarSlot});
                }
            }
        }
    }

    void InteractionSystem::expireTargets(InteractionProgress &interactionProgress, float dt) {
        auto &targets = interactionProgress.InteractionTargetList;

        for (int i = static_cast<int>(interactionProgress.InteractionTargetList.size()) - 1; i >= 0; --i) {
            auto elapsedTime = std::chrono::steady_clock::now() - targets[i].lastInteractedwith;

            double elapsed = std::chrono::duration<double>(elapsedTime).count();

            if (elapsed > 1.5) {
                targets.erase(targets.begin() + i);
            }
        }
    }
    void InteractionSystem::updateMining(Entity entity, float deltaTime, Area &area, Coordinator &coordinator, InteractionProgress &progress, const RayHit &rayHit) {
        auto &targets = progress.InteractionTargetList;
        ivec3 blockCoord = WorldToChunkArray(rayHit.hitPosition);
        ivec3 chunkPosition = WorldToChunkId(rayHit.hitPosition);
        ivec3 hitPos = rayHit.hitPosition;
        auto it = std::find_if(progress.InteractionTargetList.begin(), progress.InteractionTargetList.end(), [hitPos](const InteractionTarget &interactionTarget) {
            return interactionTarget.targetPosition == vec3(hitPos);
        });
        if (it != targets.end()) {
            double breakTime = BlockRegistry::Get().GetBlockByID(rayHit.blockID)->hardness / 1.5;

            it->progressSeconds += deltaTime; // only the block being hit
            it->percentBroken = breakTime > 0.0 ? std::clamp(it->progressSeconds / breakTime, 0.0, 1.0) : 1.0;
            it->lastInteractedwith = std::chrono::steady_clock::now();

            if (it->percentBroken >= 1.0) {
                coordinator.eventBus.blockBreakRequest.push({chunkPosition, blockCoord, entity});
                targets.erase(it);
            }
        } else if (it == progress.InteractionTargetList.end()) {
            InteractionTarget newTarget = {rayHit.hitPosition, rayHit.blockID, 0, 0, std::chrono::steady_clock::now(), std::chrono::steady_clock::now()};
            progress.InteractionTargetList.push_back(newTarget);
        }
    }
    void InteractionSystem::updatePlacing(Entity entity, float dt, Area &area, Coordinator &coord, const RayHit &hit, InventoryComponent &inv, const InputComponent &input) {}

    // InteractionTarget *InteractionSystem::findTarget(InteractionProgress &, ivec3 pos) {}
    // InteractionTarget &InteractionSystem::beginTarget(InteractionProgress &, const RayHit &) {}
} // namespace lve
