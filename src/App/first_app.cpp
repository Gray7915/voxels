#include "first_app.hpp"
#include "Rendering/Scene/MainMenu.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Rendering/Systems/chunk_render_system.hpp"
#include "Rendering/Systems/highlight_render_system.hpp"
#include "Rendering/Systems/simple_render_system.hpp"
#include "Rendering/Systems/ui_render_system.hpp"
#include "Rendering/Systems/OffScreenRenderSystem.hpp"
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

    void FirstApp::run() {
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
        if (vkCreateQueryPool(lveDevice.device(), &createInfo, nullptr, &queryPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create timestamp query pool");
        }

        lve::ECSSystems systems = registerECSComponents(coordinator);
        TextureAtlas::Get().createAtlas(); // This must run before the render setup. if it doesn't sadness will happen

        RenderSetup renderSetup = setupRender(lveDevice);

        std::cout << "setup render systems" << '\n';

        float aspect = lveRenderer.getAspectRatio();
        systems.cameraSystem->Update(aspect);

        auto currentTime = std::chrono::steady_clock::now();
        assert(lveWindow.getGLFWwindow() != nullptr && "window null)");
        AppContext context{.device = lveDevice,
                           .window = lveWindow,
                           .renderer = lveRenderer,
                           .imgui = imguiManager,
                           .renderSetup = renderSetup,
                           .coordinator = coordinator,
                           .systems = systems,
                           .stagingPool = stagingPool};
        Rendering::SceneManager sceneManager;
        sceneManager.switchTo(std::make_unique<Rendering::MenuScene>(context, sceneManager));
        sceneManager.applyPendingSwitch();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::steady_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = std::min(frameTime, 0.25f);

            aspect = lveRenderer.getAspectRatio();
            systems.cameraSystem->Update(aspect);

            sceneManager.applyPendingSwitch();
            sceneManager.current()->update(frameTime);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();

                auto start = std::chrono::steady_clock::now();
                auto end = std::chrono::steady_clock::now();

                // std::cout<< "Chunk mesh update: "<< std::chrono::duration<double, std::milli>(end - start).count()<<
                // "ms\n";
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, renderSetup.globalDescriptorSets[frameIndex]};
                sceneManager.current()->render(frameInfo);
                // std::cout << "frame in first app before set " << lveDevice.getFrameCount() << '\n';
                lveDevice.setFrameCount(lveDevice.getFrameCount() + 1);
                // std::cout << "frame in first app after set " << lveDevice.getFrameCount() << '\n';
                lveRenderer.endFrame();

                uint64_t timestamps[4];
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    };
} // namespace lve
