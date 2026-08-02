#pragma once

#include "App/RenderSetup.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Scene/GameScene.hpp"
#include "Rendering/Scene/Scene.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Ui/ImguiManager.hpp"
#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace Rendering
{
    class MenuScene : public Scene {
      public:
        MenuScene(SceneManager &sceneManager, lve::LveDevice &device, lve::LveWindow &window, lve::ImguiManager &imgui, lve::LveRenderer &renderer, lve::RenderSetup &renderSetup,
                  lve::Coordinator &coordinator, lve::ECSSystems &systems)
            : sceneManager(sceneManager), device(device), imgui(imgui), renderer(renderer), renderSetup(renderSetup), coordinator(coordinator), window(window), systems(systems) {}
        void onEnter() override {
            seedInput = 12345;
            window.setMouseActive();
        }
        void update(float deltaTime) override {
            // no logic needed for menu
        }
        void render(lve::FrameInfo &frameInfo) override {
            // no geometry pass — just UI
            renderer.StandaloneUIRenderPass->begin(frameInfo.commandBuffer, renderer.getImageIndex());
            imgui.newFrame();

            ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(400, 300), ImGuiCond_Always);
            ImGui::Begin("New World", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            ImGui::Text("Enter World Seed:");
            ImGui::InputInt("##seed", &seedInput);

            if (ImGui::Button("Generate World", ImVec2(-1, 0))) {
                // build the game scene and hand it the seed
                window.setMouseActive();
                sceneManager.switchTo(std::make_unique<GameScene>(sceneManager, device, imgui, seedInput, window, renderer, coordinator, renderSetup, systems));
            }

            ImGui::End();
            imgui.render(frameInfo.commandBuffer);
            renderer.StandaloneUIRenderPass->end(frameInfo.commandBuffer);
        }

      private:
        SceneManager &sceneManager;
        lve::RenderSetup &renderSetup;
        lve::LveDevice &device;
        lve::ImguiManager &imgui;
        lve::LveRenderer &renderer;
        lve::LveWindow &window;
        lve::Coordinator &coordinator;
        lve::ECSSystems &systems;
        int seedInput = 12345;
    };
} // namespace Rendering