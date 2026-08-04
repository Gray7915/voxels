#pragma once

#include "App/RenderSetup.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Scene/GameScene.hpp"
#include "Rendering/Scene/Scene.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Rendering/Systems/RmlRenderSystem.hpp"
#include "Ui/ImguiManager.hpp"
#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Util/AppContext.hpp"

#include <RmlUi/Core.h>
#include "Ui/RmlSystemInterface.hpp"
#include "Ui/RmlRenderInterface.hpp"

namespace Rendering
{
    class MenuScene : public Scene {
      public:
        MenuScene(AppContext &context, SceneManager &sceneManager) : sceneManager(sceneManager), context(context) {}

        void onEnter() override {
            seedInput = 19;
            context.window.setMouseActive();

            systemInterface = std::make_unique<RmlSystemInterface>();
            renderInterface = std::make_unique<lve::RmlRenderInterface>(context.device);

            Rml::SetSystemInterface(systemInterface.get());
            Rml::SetRenderInterface(renderInterface.get());
            Rml::Initialise();
            Rml::LoadFontFace("../content/ui/LatoLatin-Regular.ttf");

            rmlContext = Rml::CreateContext("main", Rml::Vector2i(context.window.getExtent().width, context.window.getExtent().height));
            rmlRenderSystem = std::make_unique<lve::RmlRenderSystem>(context.device, context.renderer.StandaloneUIRenderPass->getRenderPass(), *renderInterface);
            renderInterface->setViewportSize(context.window.getExtent().width, context.window.getExtent().height);
            document = rmlContext->LoadDocument("../content/ui/test.rml");
            if (document) {
                document->Show();
            }
        }

        void onExit() override { Rml::Shutdown(); }

        void update(float deltaTime) override {
            if (rmlContext)
                rmlContext->Update();
        }

        void render(lve::FrameInfo &frameInfo) override {
            VkExtent2D extent = context.window.getExtent();
            if (extent.width != lastWidth || extent.height != lastHeight) {
                lastWidth = extent.width;
                lastHeight = extent.height;
                if (rmlContext)
                    rmlContext->SetDimensions(Rml::Vector2i(extent.width, extent.height));
                if (renderInterface)
                    renderInterface->setViewportSize(extent.width, extent.height);
            }

            context.renderer.StandaloneUIRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            rmlRenderSystem->render(frameInfo.commandBuffer);

            if (rmlContext)
                rmlContext->Render();

            context.imgui.newFrame();
            ImVec2 displaySize = ImGui::GetIO().DisplaySize;
            ImVec2 windowSize(300.0f, 150.0f);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f), ImGuiCond_Always);
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
        int seedInput = 19;

        std::unique_ptr<RmlSystemInterface> systemInterface;
        std::unique_ptr<lve::RmlRenderInterface> renderInterface;
        std::unique_ptr<lve::RmlRenderSystem> rmlRenderSystem;
        Rml::Context *rmlContext = nullptr;
        Rml::ElementDocument *document = nullptr;

        uint32_t lastWidth = 0;
        uint32_t lastHeight = 0;
    };
} // namespace Rendering