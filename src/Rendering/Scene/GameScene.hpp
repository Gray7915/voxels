#pragma once

#include "App/RenderSetup.hpp"
#include "App/SetupECS.hpp"
#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Scene/Scene.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Rendering/Systems/chunk_render_system.hpp"
#include "Rendering/Systems/highlight_render_system.hpp"
#include "Rendering/Systems/simple_render_system.hpp"
#include "Rendering/Systems/ui_render_system.hpp"
#include "Ui/ImguiManager.hpp"
#include "Util/lve_frame_info.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkState.hpp"

#include "ECS/EntityFactory.hpp"
#include "ECS/SpawnInfo.hpp"

namespace Rendering
{
    class GameScene : public Scene {
      public:
        GameScene(SceneManager &sceneManager, lve::LveDevice &device, lve::ImguiManager &imgui, int seed, lve::LveWindow &window, lve::LveRenderer &renderer, lve::Coordinator &coordinator,
                  lve::RenderSetup &renderSetup, lve::ECSSystems &systems)
            : sceneManager(sceneManager), device(device), imgui(imgui), seed(seed), window(window), renderer(renderer), renderSetup(renderSetup), coordinator(coordinator), systems(systems) {}

        void onEnter() override {
            chunkRenderSystem.emplace(device, renderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout());
            highlightRenderSystem.emplace(device, renderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout());
            simpleRenderSystem.emplace(device, renderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout());
            // all the setup that currently happens before your game loop
            area = std::make_unique<lve::Area>(device, seed);
            // area->setSeed(seed);

            mainCamera = ECS::EntityFactory::Get().Create("MainCamera", ECS::SpawnInfo{.position = vec3{0, 68, 0}});
            // ... rest of entity setup
        }

        void onExit() override {
            // cleanup if needed when returning to menu etc.
        }

        void update(float deltaTime) override {
            lastFrameTime = deltaTime;
            auto &camTransform = coordinator.GetComponent<Transform>(mainCamera);
            auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);
            camPosition = camTransform.position;
            camRotation = camTransform.rotation;

            systems.inputSystem->Update(&window);
            systems.movementSystem->Update(deltaTime);
            systems.physicsSystem->Update(deltaTime);
            systems.collisionSystem->Update(deltaTime, *area);
            systems.interactionSystem->Update(deltaTime, window, device, *area);
            systems.inventorySystem->Update(*area);

            hoveredID = systems.interactionSystem->hoveredID;
            area->updateArea(); // gen and mutation systems updated
            coordinator.eventBus.blockBreakRequest.clear();
            coordinator.eventBus.blockPlaceRequested.clear();
        }

        void render(lve::FrameInfo &frameInfo) override {
            auto &camTransform = coordinator.GetComponent<Transform>(mainCamera);
            auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);

            area->tick(device, camPosition, frameInfo.frameIndex); // mesh gets updated here. Can't update above because don't have frame index

            // compute ray direction from stored rotation
            glm::vec3 forward = {cos(camRotation.x) * sin(camRotation.y), -sin(camRotation.x), cos(camRotation.x) * cos(camRotation.y)};
            glm::vec3 rayDir = glm::normalize(forward);

            auto block = lve::BlockRegistry::Get().GetBlockByID(hoveredID.w);
            glm::vec3 boxSize{1, 1, 1};
            if (block)
                boxSize = block->get().highlightBoxSize;

            // geometry pass
            renderer.geometryPass->begin(frameInfo.commandBuffer, renderer.getImageIndex());
            if (anyChunkReady()) {
                chunkRenderSystem->renderChunks(frameInfo, area->chunks);
                highlightRenderSystem->render(frameInfo, hoveredID.w, hoveredID, boxSize, rayDir);
            }
            renderer.geometryPass->end(frameInfo.commandBuffer);

            // ui pass
            renderer.UiRenderPass->begin(frameInfo.commandBuffer, renderer.getImageIndex());
            imgui.newFrame();
            imgui.drawDebugWindow(lastFrameTime, camPosition, camTransform, camera, *area);
            imgui.drawCrosshair(window.getExtent().width, window.getExtent().height);
            imgui.drawInv(coordinator.GetComponent<InventoryComponent>(mainCamera));
            imgui.render(frameInfo.commandBuffer);
            renderer.UiRenderPass->end(frameInfo.commandBuffer);
        }

      private:
        bool anyChunkReady() {
            for (auto &[coord, chunk] : area->chunks) {
                if (chunk->chunkState == lve::ChunkState::Uploaded && chunk->chunkModel != nullptr)
                    return true;
            }
            return false;
        }
        SceneManager &sceneManager;
        lve::LveDevice &device;
        lve::ImguiManager &imgui;
        lve::LveWindow &window;
        lve::LveRenderer &renderer;
        lve::RenderSetup &renderSetup;
        int seed;

        std::unique_ptr<lve::Area> area;
        Entity mainCamera;

        lve::Coordinator &coordinator;

        lve::ECSSystems &systems;

        std::optional<lve::ChunkRenderSystem> chunkRenderSystem;
        std::optional<lve::HighlightRenderSystem> highlightRenderSystem;
        std::optional<lve::SimpleRenderSystem> simpleRenderSystem;

        glm::ivec4 hoveredID{0};
        glm::vec3 camPosition{0};
        glm::vec3 camRotation{0};
        float lastFrameTime{0};
        // render systems, ECS systems etc.
    };
} // namespace Rendering
