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

#include "Util/AppContext.hpp"

namespace Rendering
{
    class MenuScene : public Scene {
      public:
        MenuScene(AppContext &context, SceneManager &sceneManager) : sceneManager(sceneManager), context(context) {}
        void onEnter() override {
            seedInput = 12345;
            context.window.setMouseActive();
        }
        void update(float deltaTime) override {
            // no logic needed for menu
        }
        void render(lve::FrameInfo &frameInfo) override {
            // no geometry pass — just UI
            context.renderer.StandaloneUIRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            context.imgui.newFrame();

            ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(400, 300), ImGuiCond_Always);
            ImGui::Begin("New World", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            ImGui::Text("Enter World Seed:");
            ImGui::InputInt("##seed", &seedInput);

            if (ImGui::Button("Generate World", ImVec2(-1, 0))) {
                sceneManager.switchTo(std::make_unique<GameScene>(context, sceneManager, seedInput));
            }

            ImGui::End();
            context.imgui.render(frameInfo.commandBuffer);
            context.renderer.StandaloneUIRenderPass->end(frameInfo.commandBuffer);
        }

      private:
        AppContext &context;

        SceneManager &sceneManager;
        int seedInput = 12345;
    };
} // namespace Rendering