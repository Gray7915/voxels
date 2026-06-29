#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "highlight_render_system.hpp"
#include "ui_render_system.hpp"
#include "chunk_render_system.hpp"

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

namespace lve
{
    Coordinator coordinator;

    FirstApp::FirstApp() : area(gameObjects, lveDevice, glm::vec3(0, 0, 0)), hoveredID(glm::ivec4(0, 0, 0, 0))
    {
        std::cout << "do we construct?" << '\n';

        globalPool = LveDescriptorPool::Builder(lveDevice)
                         .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();
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

        Entity mainCamera = coordinator.CreateEntity();
        coordinator.AddComponent(mainCamera, Transform{.position = {0, 70, 0}});
        coordinator.AddComponent(mainCamera, GravityComponent{glm::vec3(0.0f, -15, 0.0f)});
        coordinator.AddComponent(mainCamera, RigidBodyComponent{.velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)});
        coordinator.AddComponent(mainCamera, CameraComponent{});
        coordinator.AddComponent(mainCamera, InputComponent{});
        coordinator.AddComponent(mainCamera, MovementStats{.moveSpeed = 6.5f, .jumpForce = 5.8});
        coordinator.AddComponent(mainCamera, AABBComponent{.halfExtents = glm::vec3(0.4, 0.8, 0.4)});

        float aspect = lveRenderer.getAspectRatio();
        systems.cameraSystem->Update(aspect);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::cout << "got to while loop" << '\n';
        assert(lveWindow.getGLFWwindow() != nullptr && "window null)");
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();
            // std::cout << "polled glfw" << '\n';

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            // std::cout << "got to timesetting" << '\n';

            systems.inputSystem->Update(&lveWindow);
            systems.movementSystem->Update(frameTime);
            systems.physicsSystem->Update(frameTime);
            systems.collisionSystem->Update(frameTime);
            auto &camCollision = coordinator.GetComponent<AABBComponent>(mainCamera);
            auto &camBody = coordinator.GetComponent<RigidBodyComponent>(mainCamera);

            if (camCollision.isTrigger)
            {
                camBody.velocity.y = 0;
            }
            // std::cout << "updated movement and input" << '\n';

            aspect = lveRenderer.getAspectRatio();
            systems.cameraSystem->Update(aspect);

            auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);

            float aspect = lveRenderer.getAspectRatio();

            auto &camTransform = coordinator.GetComponent<Transform>(mainCamera);
            // std::cout << "got to getting camera transform" << '\n';
            // std::cout << "camera coordinate " << " " << camTransform.position.x << " " << camTransform.position.y << " " << camTransform.position.z << '\n';

            glm::vec3 rot = camTransform.rotation;
            glm::vec3 forward = {cos(rot.x) * sin(rot.y), -sin(rot.x), cos(rot.x) * cos(rot.y)};

            glm::vec3 rayDir = glm::normalize(forward);

            Ray ray((camTransform.position + camera.relativePosition), rayDir);
            glm::ivec3 rayHit = ray.detectBlockHit(4.0f); // worldspace
            glm::ivec3 chunkPos = glm::ivec3(camTransform.position) / glm::ivec3(16, 32, 16);

            if (rayHit == glm::ivec3(-1.0f))
            {
                hoveredID = glm::ivec4(-1.0f);
            }
            else
            {
                hoveredID = glm::ivec4(rayHit, 1);
            }

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    renderSetup.globalDescriptorSets[frameIndex]};

                // update
                camera.projectionMatrix[1][1] *= -1;

                GlobalUbo ubo{};
                ubo.projectionView = camera.projectionMatrix * camera.viewMatrix;
                renderSetup.uboBuffers[frameIndex]->writeToBuffer(&ubo);
                renderSetup.uboBuffers[frameIndex]->flush();

                // render pass START
                lveRenderer.geometryPass->begin(commandBuffer, lveRenderer.getImageIndex());

                chunkRenderSystem.renderChunks(frameInfo, Area::chunks);
                // simpleRenderSystem.renderGameObjects(frameInfo, gameObjects, hoveredID);
                highlightRenderSystem.render(frameInfo, hoveredID.w != 0, glm::ivec3(hoveredID), rayDir);
                // uiRenderSystem.renderUI(frameInfo);

                // render pass END
                lveRenderer.geometryPass->end(commandBuffer);

                lveRenderer.UiRenderPass->begin(commandBuffer, lveRenderer.getImageIndex());
                imguiManager->newFrame();
                // imguiManager->drawDebugWindow(frameTime);
                imguiManager->drawCrosshair(WIDTH, HEIGHT);
                imguiManager->drawQuitMenu(WIDTH, HEIGHT);
                imguiManager->render(commandBuffer);
                lveRenderer.UiRenderPass->end(commandBuffer);

                lveRenderer.endFrame();
            }

            static bool pWasPressed = false;
            bool pIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
            if (pWasPressed && !pIsPressed)
                pWasPressed = false;

            if (pIsPressed && !pWasPressed && rayHit != glm::ivec3(-1.0f))
            {
                pWasPressed = true;
                std::cout << "Ray Hit " << rayHit.x << " " << rayHit.y << " " << rayHit.z << '\n';
                // std::cout << "Ray Orgin " << viewerObject.transform.translation.x << " " << viewerObject.transform.translation.y << " " << viewerObject.transform.translation.z << '\n';
                // std::cout << "Ray Direction " << forward.x << " " << forward.y << " " << forward.z << '\n';
                glm::ivec3 blockCoord = WorldToChunkArray(rayHit);
                glm::ivec3 chunkPosition = WorldToChunkId(rayHit);
                Area::chunks.find(chunkPosition)->second->blocks[blockCoord.x][blockCoord.y][blockCoord.z] = 0;
                // std::cout << "array place found" << '\n';
                // gameObjects.find(chunkPos)->second.model.reset();
                Area::chunks.find(chunkPosition)->second->chunkModel = ChunkRenderer::mesh(Area::chunks.find(chunkPosition)->second->blocks, lveDevice, {0, 0, 0});

                // std::cout << "chunk changed" << '\n';

                vkDeviceWaitIdle(lveDevice.device());
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

    void FirstApp::loadGameObjects()
    {
        // LveGameObject chunk = Chunk::createChunk();
        // std::cout << "load game objects" << '\n';
        area = Area(gameObjects, lveDevice, glm::vec3(0, 0, 0));
        // chunk::createChunk(&gameObjects, lveDevice);
    }
}
