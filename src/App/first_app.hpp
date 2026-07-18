#pragma once

#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Rendering/Core/lve_renderer.hpp"
#include "Rendering/Core/lve_descriptors.hpp"
#include "Rendering/Core/lve_Texture.hpp"
#include "Rendering/Passes/GeometryPass.hpp"

#include <unordered_map>

#include "Ui/ImguiManager.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/InputSystem.hpp"
#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"
#include "ECS/Systems/InteractionSystem.hpp"

#include "World/Area.hpp"
#include "World/Systems/ChunkMeshSystem.hpp"
#include "World/Systems/ChunkGenerationSystem.hpp"
#include "World/Systems/ChunkMutationSystem.hpp"

#include "Util/lve_frame_info.hpp"
#include "Util/ray.hpp"
#include "Util/IVec3Hash.h"
#include "Util/lve_util.hpp"

#include "SetupECS.hpp"
#include "RenderSetup.hpp"
// std

#include <memory>
#include <vector>

#include "App/TextureAtlas.hpp"

namespace lve
{
    class FirstApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        std::unique_ptr<LveTexture> texture;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void createPipelineLayout();
        void createPipeline();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        // void registerECSComponents();
        // bool getTargetBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, glm::ivec3 &out);

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        // std::vector<LveGameObject> gameObjects;
        // std::unordered_map<glm::vec3, LveGameObject> gameObjects;
        std::unique_ptr<LveDescriptorPool> globalPool{};
        Area area;
        // glm::ivec4 hoveredID;

        ECSSystems systems;
        RenderSetup renderSetup;
        std::unique_ptr<ImguiManager> imguiManager;

        ChunkGenerationSystem chunkGenSystem{area};
        ChunkMeshSystem chunkMeshSystem{area, lveDevice};
        ChunkMutationSystem chunkMutationSystem{};

        VkQueryPool queryPool;

        TextureAtlas atlas{};
    };
}
