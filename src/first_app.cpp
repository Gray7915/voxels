#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "highlight_render_system.hpp"
#include "chunk_render_system.hpp"
#include "lve_util.hpp"

// #include "Input/keyboard_movement_controller.hpp"
#include "IVec3Hash.h"
#include "World/cube.hpp"
#include "World/Chunk.hpp"
#include "Util/ray.hpp"
#include "World/ChunkRenderer.hpp"
#include "lve_buffer.hpp"
#include "ECS/Coordinator.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <optional>
#include <cassert>

#include "SetupECS.hpp"

#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/MovementStats.hpp"
#include "ECS/Components/ColliderComponent.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Renderable.hpp"

namespace lve
{
    Coordinator coordinator;

    FirstApp::FirstApp() : area(lveDevice, glm::vec3(0, 0, 0))
    {
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        coordinator.Init();
        auto systems = registerECSComponents(coordinator);
        auto renderSetup = setupRender(lveDevice);

        imguiManager = std::make_unique<ImguiManager>(lveDevice, lveWindow, lveRenderer);

        HighlightRenderSystem highlightRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        ChunkRenderSystem chunkRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};

        Entity mainCamera = coordinator.CreateEntity();
        coordinator.AddComponent(mainCamera, Transform{.position = {0, 68, 0}});
        coordinator.AddComponent(mainCamera, GravityComponent{glm::vec3(0.0f, -15, 0.0f)});
        coordinator.AddComponent(mainCamera, RigidBodyComponent{.velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)});
        coordinator.AddComponent(mainCamera, CameraComponent{});
        coordinator.AddComponent(mainCamera, InputComponent{});
        coordinator.AddComponent(mainCamera, MovementStats{.moveSpeed = 6.5f, .jumpForce = 6.1});
        coordinator.AddComponent(mainCamera, AABBComponent{.halfExtents = glm::vec3(0.4, 0.8, 0.4)});

        float aspect = lveRenderer.getAspectRatio();
        systems.cameraSystem->Update(aspect);
        auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);
        camera.projectionMatrix[1][1] *= -1;

        Entity testEntity = coordinator.CreateEntity();
        coordinator.AddComponent(testEntity, Transform{.position = {0, 66, 0}, .scale = {1, 1, 1}});
        coordinator.AddComponent(testEntity, RenderableComponent{.model = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj")});
        //    auto &testModel = coordinator.GetComponent<RenderableComponent>(testEntity);

        auto currentTime = std::chrono::high_resolution_clock::now();
        assert(lveWindow.getGLFWwindow() != nullptr && "window null)");
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            systems.inputSystem->Update(&lveWindow);
            systems.movementSystem->Update(frameTime);
            systems.physicsSystem->Update(frameTime);
            systems.collisionSystem->Update(frameTime);
            systems.interactionSystem->Update(frameTime, lveWindow, lveDevice);

            aspect = lveRenderer.getAspectRatio();
            systems.cameraSystem->Update(aspect);

            auto &camCollision = coordinator.GetComponent<AABBComponent>(mainCamera);
            auto &camBody = coordinator.GetComponent<RigidBodyComponent>(mainCamera);
            auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);
            auto &camTransform = coordinator.GetComponent<Transform>(mainCamera);

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();

                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, renderSetup.globalDescriptorSets[frameIndex]};

                // flip so pos y is up and neg y is downs
                camera.projectionMatrix[1][1] *= -1;

                GlobalUbo ubo{};
                ubo.projectionView = camera.projectionMatrix * camera.viewMatrix;
                ubo.lightPosition = camTransform.position;
                renderSetup.uboBuffers[frameIndex]->writeToBuffer(&ubo);
                renderSetup.uboBuffers[frameIndex]->flush();

                lveRenderer.geometryPass->begin(commandBuffer, lveRenderer.getImageIndex());

                chunkRenderSystem.renderChunks(frameInfo, Area::chunks);
                // systems.renderSystem->Update(frameInfo, simpleRenderSystem);
                highlightRenderSystem.render(frameInfo, systems.interactionSystem->hoveredID.w != 0, systems.interactionSystem->hoveredID);
                auto &testTrans = coordinator.GetComponent<Transform>(testEntity);
                auto &testModel = coordinator.GetComponent<RenderableComponent>(testEntity);

                simpleRenderSystem.renderGameObjects(frameInfo, testTrans.mat4(), testTrans.normalMatrix(), testModel.model);

                lveRenderer.geometryPass->end(commandBuffer);

                lveRenderer.UiRenderPass->begin(commandBuffer, lveRenderer.getImageIndex());
                imguiManager->newFrame();
                imguiManager->drawDebugWindow(frameTime, camTransform.position);
                imguiManager->drawCrosshair(WIDTH, HEIGHT);
                // imguiManager->drawQuitMenu(WIDTH, HEIGHT);
                imguiManager->render(commandBuffer);
                lveRenderer.UiRenderPass->end(commandBuffer);

                lveRenderer.endFrame();
            }

            static bool colWasPressed = false;
            bool colIsPressed = glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_P) == GLFW_PRESS;
            if (colWasPressed && !colIsPressed)
                colWasPressed = false;

            if (colIsPressed && !colWasPressed)
            {
                camCollision.collisionEnabled = !camCollision.collisionEnabled;
                std::cout << "collision enabled / disabled" << '\n';
            }

            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                lveWindow.setMouseActive();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    };
}
