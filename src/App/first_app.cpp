#include "first_app.hpp"
#include "Rendering/Scene/MainMenu.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Rendering/Systems/chunk_render_system.hpp"
#include "Rendering/Systems/highlight_render_system.hpp"
#include "Rendering/Systems/simple_render_system.hpp"
#include "Rendering/Systems/ui_render_system.hpp"
#include "World/Chunk.hpp"

#include "ECS/Coordinator.hpp"
#include "Rendering/Core/lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <di.hpp>
namespace di = boost::di;

#include "Util/Types.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "App/BlockRegistrySetup.hpp"
#include "App/EntityFactorySetup.hpp"
#include "App/ItemRegistrySetup.hpp"
#include "SetupECS.hpp"

#include "ECS/Components/Camera.hpp"
#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/MovementStats.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Transform.hpp"
// #include "ECS/Components/ColliderComponent.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Components/Renderable.hpp"
#include "ECS/SpawnInfo.hpp"

#include "Inventory/ItemRegistry.hpp"
#include "World/Blocks/BlockRegistry.hpp"

namespace lve
{
    Coordinator coordinator;
    // test
    FirstApp::FirstApp() {}

    FirstApp::~FirstApp() { vkDestroyQueryPool(lveDevice.device(), queryPool, nullptr); }

    void FirstApp::run()
    {
        auto injector = di::make_injector(di::bind<LveDevice>.to(lveDevice), di::bind<Coordinator>.to(coordinator), di::bind<LveWindow>.to(lveWindow), di::bind<LveRenderer>.to(lveRenderer),
                                          di::bind<BlockRegistry>.to(BlockRegistry::Get()), di::bind<ItemRegistry>.to(ItemRegistry::Get()));

        coordinator.Init();
        ItemRegistrySetup::SetupItemRegistry(ItemRegistry::Get());
        BlockRegistrySetup::SetupBlockRegistry(BlockRegistry::Get(), lveDevice);
        EntityFactorySetup::SetupEntityFactory(ECS::EntityFactory::Get(), coordinator);

        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = 16;
        if (vkCreateQueryPool(lveDevice.device(), &createInfo, nullptr, &queryPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create timestamp query pool");
        }

        lve::ECSSystems systems = registerECSComponents(coordinator);
        TextureAtlas::Get().createAtlas(); // This must run before the render setup. if it doesn't sadness will happen

        RenderSetup renderSetup = setupRender(lveDevice);

        std::cout << "setup systems" << '\n';
        HighlightRenderSystem highlightRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        ChunkRenderSystem chunkRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), renderSetup.globalSetLayout->getDescriptorSetLayout()};
        std::cout << "setup render systems" << '\n';

        float aspect = lveRenderer.getAspectRatio();
        systems.cameraSystem->Update(aspect);

        auto currentTime = std::chrono::high_resolution_clock::now();
        assert(lveWindow.getGLFWwindow() != nullptr && "window null)");
        AppContext context{.device = lveDevice, .window = lveWindow, .renderer = lveRenderer, .imgui = imguiManager, .renderSetup = renderSetup, .coordinator = coordinator, .systems = systems, .stagingPool = stagingPool};
        Rendering::SceneManager sceneManager;
        sceneManager.switchTo(std::make_unique<Rendering::MenuScene>(context, sceneManager));
        sceneManager.applyPendingSwitch();

        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            // std::cout << "Set time in loop" << '\n';
            // coordinator.eventBus.blockBreakRequest.clear();
            // coordinator.eventBus.blockPlaceRequested.clear();

            aspect = lveRenderer.getAspectRatio();
            systems.cameraSystem->Update(aspect);

            sceneManager.applyPendingSwitch();
            sceneManager.current()->update(frameTime);

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();

                auto start = std::chrono::high_resolution_clock::now();
                auto end = std::chrono::high_resolution_clock::now();

                // std::cout<< "Chunk mesh update: "<< std::chrono::duration<double, std::milli>(end - start).count()<<
                // "ms\n";
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, renderSetup.globalDescriptorSets[frameIndex]};
                sceneManager.current()->render(frameInfo);
                lveDevice.setFrameCount(lveDevice.getFrameCount() + 1);
                /*
                            auto &camera = coordinator.GetComponent<CameraComponent>(mainCamera);


                            vkCmdResetQueryPool(commandBuffer, queryPool, 0, 8);

                            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);

                            lveRenderer.geometryPass->begin(commandBuffer, lveRenderer.getImageIndex());
                            auto newstart = std::chrono::high_resolution_clock::now();
                            chunkRenderSystem.renderChunks(frameInfo, area.chunks);

                            auto newms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - newstart).count();
                            // if (newms > 2.0)
                            // std::cout << "[HITCH] loadChunk took " << newms << "ms\n";
                            // systems.renderSystem->Update(frameInfo, simpleRenderSystem);
                            auto block = BlockRegistry::Get().GetBlockByID(systems.interactionSystem->hoveredID.w);
                            glm::vec3 boxSize{1, 1, 1};
                            if (block)
                                boxSize = block->get().highlightBoxSize;
                            // std::cout << "highlightedboxsize" << boxSize.x << " " << boxSize.y << " " << boxSize.z << '\n';

                            vec3 rot = camTransform.rotation;
                            vec3 forward = {cos(rot.x) * sin(rot.y), -sin(rot.x), cos(rot.x) * cos(rot.y)};
                            vec3 rayDir = glm::normalize(forward);

                            auto &testTrans = coordinator.GetComponent<Transform>(testEntity);
                            auto &testModel = coordinator.GetComponent<RenderableComponent>(testEntity);
                            simpleRenderSystem.renderGameObjects(frameInfo, testTrans.mat4(), testTrans.normalMatrix(), testModel.model);

                            // systems.renderSystem->Update(frameInfo, simpleRenderSystem);
                            highlightRenderSystem.render(frameInfo, systems.interactionSystem->hoveredID.w != 0, systems.interactionSystem->hoveredID, boxSize, rayDir,
                                                         BlockRegistry::Get().GetBlockByID(systems.interactionSystem->hoveredID.w)->get().highlightShape);

                            lveRenderer.geometryPass->end(commandBuffer);

                            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 1);
                            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 2);

                            lveRenderer.UiRenderPass->begin(commandBuffer, lveRenderer.getImageIndex());
                            imguiManager.newFrame();
                            imguiManager.drawDebugWindow(frameTime, camTransform.position, camTransform, coordinator.GetComponent<CameraComponent>(mainCamera), area);
                            imguiManager.drawCrosshair(lveWindow.getExtent().width, lveWindow.getExtent().height);
                            imguiManager.drawInv(coordinator.GetComponent<InventoryComponent>(mainCamera));
                            // imguiManager->drawQuitMenu(WIDTH, HEIGHT);
                            imguiManager.render(commandBuffer);
                            lveRenderer.UiRenderPass->end(commandBuffer);

                            vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 3);
            */
                lveRenderer.endFrame();

                uint64_t timestamps[4];

                // vkGetQueryPoolResults(lveDevice.device(), queryPool, 0, 4, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

                // double geometryMs = (timestamps[1] - timestamps[0]) * lveDevice.getTimestampPeriod() / 1'000'000.0;
                // double uiMs = (timestamps[3] - timestamps[2]) * lveDevice.getTimestampPeriod() / 1'000'000.0;

                //  std::cout << "Chunks: " << area.chunks.size() << " Geometry: " << geometryMs << "\n";
                // std::cout << "UI Pass time " << uiMs << '\n';
                // std::cout << frameTime << "\n";
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
} // namespace lve
