#include "first_app.hpp"
#include "Rendering/Systems/chunk_render_system.hpp"
#include "Rendering/Systems/highlight_render_system.hpp"
#include "Rendering/Systems/ui_render_system.hpp"
#include "Rendering/Systems/simple_render_system.hpp"

#include "World/Chunk.hpp"

#include "Rendering/Core/lve_buffer.hpp"
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
#include "App/ItemRegistrySetup.hpp"
#include "App/BlockRegistrySetup.hpp"

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
#include "ECS/Components/InventoryComponent.hpp"

#include "Inventory/ItemRegistry.hpp"
#include "World/Blocks/BlockRegistry.hpp"

namespace lve
{
    Coordinator coordinator;
    // test
    FirstApp::FirstApp() : area(lveDevice, glm::vec3(0, 0, 0), chunkGenSystem)
    {
    }

    FirstApp::~FirstApp()
    {
        vkDestroyQueryPool(lveDevice.device(), queryPool, nullptr);
    }

    void FirstApp::run()
    {
        coordinator.Init();
        ItemRegistrySetup::SetupItemRegistry(ItemRegistry::Get());
        BlockRegistrySetup::SetupBlockRegistry(BlockRegistry::Get(), lveDevice);

        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = 16;
        if (vkCreateQueryPool(lveDevice.device(), &createInfo, nullptr, &queryPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create timestamp query pool");
        }

        auto systems = registerECSComponents(coordinator);
        TextureAtlas::Get().createAtlas(); // This must run before the render setup. if it doesn't sadness will happen
        auto renderSetup = setupRender(lveDevice);
        std::cout << "setup systems" << '\n';
        HighlightRenderSystem highlightRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        ChunkRenderSystem chunkRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        std::cout << "setup render systems" << '\n';

        Entity mainCamera = coordinator.CreateEntity();
        coordinator.AddComponent(mainCamera, Transform{.position = {0, 68, 0}});
        coordinator.AddComponent(mainCamera, GravityComponent{glm::vec3(0.0f, -15.0f, 0.0f)});
        coordinator.AddComponent(mainCamera, RigidBodyComponent{.velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)});
        coordinator.AddComponent(mainCamera, CameraComponent{});
        coordinator.AddComponent(mainCamera, InputComponent{});
        coordinator.AddComponent(mainCamera, MovementStats{.moveSpeed = 6.5f, .jumpForce = 6.1});
        coordinator.AddComponent(mainCamera, AABBComponent{.halfExtents = glm::vec3(0.4, 0.8, 0.4)});
        coordinator.AddComponent(mainCamera, InventoryComponent{});
        std::cout << "create camera entity" << '\n';

        float aspect = lveRenderer.getAspectRatio();
        systems.cameraSystem->Update(aspect);

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
            // std::cout << "Set time in loop" << '\n';

            systems.inputSystem->Update(&lveWindow);
            systems.movementSystem->Update(frameTime);
            systems.physicsSystem->Update(frameTime);
            systems.collisionSystem->Update(frameTime, area);
            systems.interactionSystem->Update(frameTime, lveWindow, lveDevice, area);
            systems.inventorySystem->Update(area);

            chunkMutationSystem.Update(area);
            chunkGenSystem.update();
            coordinator.eventBus.blockBreakRequest.clear();
            coordinator.eventBus.blockPlaceRequested.clear();

            aspect = lveRenderer.getAspectRatio();
            systems.cameraSystem->Update(aspect);

            auto &camCollision = coordinator.GetComponent<AABBComponent>(mainCamera);
            auto &camTransform = coordinator.GetComponent<Transform>(mainCamera);

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();
                auto start = std::chrono::high_resolution_clock::now();

                chunkMeshSystem.Update(lveDevice, frameIndex);

                auto end = std::chrono::high_resolution_clock::now();

                std::cout
                    << "Chunk mesh update: "
                    << std::chrono::duration<double, std::milli>(end - start).count()
                    << "ms\n";
                area.tick(lveDevice, camTransform.position, frameIndex, chunkGenSystem);

                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, renderSetup.globalDescriptorSets[frameIndex]};
                auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);
                GlobalUbo ubo{};
                ubo.projectionView = camera.projectionMatrix * camera.viewMatrix;
                // ubo.lightPosition = camTransform.position;
                ubo.cameraPosition = glm::ivec4(camTransform.position, 1);
                renderSetup.uboBuffers[frameIndex]->writeToBuffer(&ubo);
                renderSetup.uboBuffers[frameIndex]->flush();

                vkCmdResetQueryPool(commandBuffer, queryPool, 0, 8);

                vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);

                lveRenderer.geometryPass->begin(commandBuffer, lveRenderer.getImageIndex());

                chunkRenderSystem.renderChunks(frameInfo, area.chunks);
                // systems.renderSystem->Update(frameInfo, simpleRenderSystem);
                auto block = BlockRegistry::Get().GetBlockByID(systems.interactionSystem->hoveredID.w);
                glm::vec3 boxSize{1, 1, 1};
                if (block)
                    boxSize = block->get().highlightBoxSize;
                // std::cout << "highlightedboxsize" << boxSize.x << " " << boxSize.y << " " << boxSize.z << '\n';

                highlightRenderSystem.render(frameInfo, systems.interactionSystem->hoveredID.w != 0, systems.interactionSystem->hoveredID, boxSize);
                auto &testTrans = coordinator.GetComponent<Transform>(testEntity);
                auto &testModel = coordinator.GetComponent<RenderableComponent>(testEntity);

                simpleRenderSystem.renderGameObjects(frameInfo, testTrans.mat4(), testTrans.normalMatrix(), testModel.model);

                lveRenderer.geometryPass->end(commandBuffer);

                vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 1);
                vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 2);

                lveRenderer.UiRenderPass->begin(commandBuffer, lveRenderer.getImageIndex());
                imguiManager.newFrame();
                imguiManager.drawDebugWindow(frameTime, camTransform.position);
                imguiManager.drawCrosshair(lveWindow.getExtent().width, lveWindow.getExtent().height);
                imguiManager.drawInv(coordinator.GetComponent<InventoryComponent>(mainCamera));
                // imguiManager->drawQuitMenu(WIDTH, HEIGHT);
                imguiManager.render(commandBuffer);
                lveRenderer.UiRenderPass->end(commandBuffer);

                vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 3);
                lveRenderer.endFrame();

                uint64_t timestamps[4];

                vkGetQueryPoolResults(lveDevice.device(), queryPool, 0, 4, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

                double geometryMs = (timestamps[1] - timestamps[0]) * lveDevice.getTimestampPeriod() / 1'000'000.0;
                double uiMs = (timestamps[3] - timestamps[2]) * lveDevice.getTimestampPeriod() / 1'000'000.0;
                //  std::cout << "Chunks: " << area.chunks.size() << " Geometry: " << geometryMs << "\n";
                // std::cout << "UI Pass time " << uiMs << '\n';
                std::cout << frameTime << "\n";
            }

            static bool colWasPressed = false;
            bool colIsPressed = glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_P) == GLFW_PRESS;
            if (colWasPressed && !colIsPressed)
                colWasPressed = false;

            if (colIsPressed && !colWasPressed)
            {
                camCollision.collisionEnabled = !camCollision.collisionEnabled;
                // std::cout << "collision enabled / disabled" << '\n';
            }
            /*

                        if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
                        {
                            lveWindow.setMouseActive();
                        }
            */
        }
        vkDeviceWaitIdle(lveDevice.device());
    };
}
