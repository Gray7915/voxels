#pragma once
#include <functional>
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
#include "Util/ButtonListener.hpp"

#include <RmlUi/Core.h>
#include "Ui/RmlSystemInterface.hpp"
#include "Ui/RmlRenderInterface.hpp"

namespace Rendering
{
    class MenuScene : public Scene {
      public:
        MenuScene(AppContext &context, SceneManager &sceneManager) : sceneManager(sceneManager), context(context) {}

        void onEnter() override {
            context.window.setMouseActive();

            systemInterface = std::make_unique<RmlSystemInterface>();
            renderInterface = std::make_unique<lve::RmlRenderInterface>(context.device);

            Rml::SetSystemInterface(systemInterface.get());
            Rml::SetRenderInterface(renderInterface.get());
            Rml::Initialise();
            Rml::LoadFontFace("../content/ui/LatoLatin-Regular.ttf");

            rmlContext = Rml::CreateContext("main", Rml::Vector2i(context.window.getExtent().width, context.window.getExtent().height));
            context.window.setRmlContext(rmlContext);
            rmlRenderSystem = std::make_unique<lve::RmlRenderSystem>(context.device, context.renderer.StandaloneUIRenderPass->getRenderPass(), *renderInterface);
            renderInterface->setViewportSize(context.window.getExtent().width, context.window.getExtent().height);

            if (Rml::DataModelConstructor constructor = rmlContext->CreateDataModel("menu")) {
                constructor.Bind("seed", &seedText);
                menuModel = constructor.GetModelHandle();
            }

            document = rmlContext->LoadDocument("../content/ui/test.rml");
            if (document) {
                document->Show();
                auto button = document->GetElementById("generate_button");
                buttonListener = std::make_unique<ButtonListener>([this]() {
                    std::cout << "seedText = " << seedText << "\n";
                    std::cout << "seed as int = " << std::stoi(seedText) << '\n';
                    sceneManager.switchTo(std::make_unique<GameScene>(context, sceneManager, std::stoi(seedText)));
                });

                button->AddEventListener(Rml::EventId::Click, buttonListener.get());
            }
        }

        void onExit() override {
            if (rmlContext) {
                rmlContext->UnloadAllDocuments();
                Rml::RemoveContext(rmlContext->GetName());
                rmlContext = nullptr;
            }

            Rml::Shutdown();
        }

        void update(float deltaTime) override {
            if (rmlContext) {
                if (menuModel.IsVariableDirty("seed")) {
                    // seedText has already been updated by RmlUi, nothing extra needed
                    // but you must call this to clear the dirty flag
                    menuModel.DirtyVariable("seed");
                }
                rmlContext->Update();
            }
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

            if (rmlContext) {
                renderInterface->setFrameIndex(frameInfo.frameIndex);
                rmlContext->Render();
            }

            context.renderer.StandaloneUIRenderPass->end(frameInfo.commandBuffer);
        }

      private:
        AppContext &context;
        SceneManager &sceneManager;
        Rml::String seedText = "19";
        std::unique_ptr<RmlSystemInterface> systemInterface;
        std::unique_ptr<lve::RmlRenderInterface> renderInterface;
        std::unique_ptr<lve::RmlRenderSystem> rmlRenderSystem;
        std::unique_ptr<ButtonListener> buttonListener;

        Rml::Context *rmlContext = nullptr;
        Rml::ElementDocument *document = nullptr;
        Rml::DataModelHandle menuModel;
        uint32_t lastWidth = 0;
        uint32_t lastHeight = 0;
    };
} // namespace Rendering