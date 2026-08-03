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
#include "Util/AppContext.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkState.hpp"

#include "ECS/EntityFactory.hpp"
#include "ECS/SpawnInfo.hpp"

#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace Rendering
{
    enum class GameSceneState { Loading, Playing };
    class GameScene : public Scene {
      public:
        GameScene(AppContext &context, SceneManager &sceneManager, int seed) : context(context), sceneManager(sceneManager), seed(seed) {}

        void onEnter() override {
            chunkRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            highlightRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            simpleRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            area = std::make_unique<lve::Area>(context.device, seed);
        }

        void onExit() override {
            // cleanup if needed when returning to menu etc.
        }

        void update(float deltaTime) override {
            area->updateArea();

            if (state == GameSceneState::Loading) {
                return;
            }

            lastFrameTime = deltaTime;
            auto &camTransform = context.coordinator.GetComponent<Transform>(mainCamera);
            camPosition = camTransform.position;
            camRotation = camTransform.rotation;

            context.systems.inputSystem->Update(&context.window);
            context.systems.movementSystem->Update(deltaTime);
            context.systems.physicsSystem->Update(deltaTime);
            context.systems.collisionSystem->Update(deltaTime, *area);
            context.systems.interactionSystem->Update(deltaTime, context.window, context.device, *area);
            context.systems.inventorySystem->Update(*area);

            hoveredID = context.systems.interactionSystem->hoveredID;
            context.coordinator.eventBus.blockBreakRequest.clear();
            context.coordinator.eventBus.blockPlaceRequested.clear();
        }

        void render(lve::FrameInfo &frameInfo) override {
            area->tick(context.device, camPosition, frameInfo.frameIndex);
            if (state == GameSceneState::Loading) {
                if (chunksReady() >= minimumChunksToLoad) {
                    state = GameSceneState::Playing;
                    context.window.setMouseActive();

                    vec3 spawnPos = findSpawnPosition({0, 0});
                    camPosition = spawnPos;
                    mainCamera = ECS::EntityFactory::Get().Create("MainCamera", ECS::SpawnInfo{.position = spawnPos, .scale{1, 1, 1}});
                } else {
                    renderLoadingScreen(frameInfo);
                    return;
                }
            }

            auto &camTransform = context.coordinator.GetComponent<Transform>(mainCamera);
            auto &camera = context.coordinator.GetComponent<CameraComponent>(mainCamera);

            context.renderer.geometryPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            chunkRenderSystem->renderChunks(frameInfo, area->chunks);
            if (hoveredID.w != 0) {
                auto block = lve::BlockRegistry::Get().GetBlockByID(hoveredID.w);
                vec3 boxSize{1, 1, 1};
                if (block)
                    boxSize = block->get().highlightBoxSize;
                vec3 forward = {cos(camRotation.x) * sin(camRotation.y), -sin(camRotation.x), cos(camRotation.x) * cos(camRotation.y)};
                vec3 rayDir = glm::normalize(forward);
                highlightRenderSystem->render(frameInfo, hoveredID.w, hoveredID, boxSize, rayDir);
            }
            context.renderer.geometryPass->end(frameInfo.commandBuffer);

            renderUI(frameInfo, camTransform, camera);

            lve::GlobalUbo ubo{};
            ubo.projectionView = camera.projectionMatrix * camera.viewMatrix;
            ubo.cameraPosition = ivec4(camTransform.position, 1);
            context.renderSetup.uboBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
            context.renderSetup.uboBuffers[frameInfo.frameIndex]->flush();
        }

      private:
        static constexpr int minimumChunksToLoad = 9;

        int chunksReady() {
            auto spawnChunk = area->chunks.find(ivec3{0, 0, 0});
            if (spawnChunk != area->chunks.end()) {
                if (!spawnChunk->second->voxelData.isGenerated()) {
                    return 0;
                }
            }

            int count = 0;
            for (auto &[coord, chunk] : area->chunks)
                if (chunk && chunk->chunkState == lve::ChunkState::Uploaded && chunk->chunkModel != nullptr)
                    count++;
            return count;
        }

        vec3 findSpawnPosition(vec2 xzPos) {
            for (int y = 127; y >= 0; y--) {
                if (area->isBlockSolid({xzPos.x, y, xzPos.y})) {
                    return vec3{xzPos.x + 0.5f, y + 2, xzPos.y + 0.5f};
                }
            }
            return vec3{xzPos.x, 68, xzPos.y};
        }

        void renderLoadingScreen(lve::FrameInfo &frameInfo) {
            context.renderer.StandaloneUIRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            context.imgui.newFrame();

            ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(400, 300), ImGuiCond_Always);
            ImGui::Begin("##loading", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
            ImGui::Text("Generating world... %d / %d", chunksReady(), minimumChunksToLoad);
            ImGui::End();

            context.imgui.render(frameInfo.commandBuffer);
            context.renderer.StandaloneUIRenderPass->end(frameInfo.commandBuffer);
        }

        void renderUI(lve::FrameInfo &frameInfo, Transform &camTransform, CameraComponent &camera) {
            context.renderer.UiRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            context.imgui.newFrame();
            context.imgui.drawDebugWindow(lastFrameTime, camPosition, camTransform, camera, *area);
            context.imgui.drawCrosshair(context.window.getExtent().width, context.window.getExtent().height);
            context.imgui.drawInv(context.coordinator.GetComponent<InventoryComponent>(mainCamera));
            context.imgui.render(frameInfo.commandBuffer);
            context.renderer.UiRenderPass->end(frameInfo.commandBuffer);
        }

        AppContext &context;
        SceneManager &sceneManager;
        int seed;

        std::unique_ptr<lve::Area> area;
        Entity mainCamera;

        std::optional<lve::ChunkRenderSystem> chunkRenderSystem;
        std::optional<lve::HighlightRenderSystem> highlightRenderSystem;
        std::optional<lve::SimpleRenderSystem> simpleRenderSystem;

        ivec4 hoveredID{0};
        vec3 camPosition{0};
        vec3 camRotation{0};
        float lastFrameTime{0};

        GameSceneState state = GameSceneState::Loading;
    };
} // namespace Rendering
