#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include <unordered_map>
#include "IVec3Hash.h"
#include "World/Area.hpp"
#include "lve_descriptors.hpp"
#include "lve_Texture.hpp"
#include "GeometryPass.hpp"
#include "ImguiManager.hpp"

#include "ECS/Coordinator.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/InputSystem.hpp"
#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"
// std

#include <memory>
#include <vector>

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
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        void registerECSComponents();
        bool getTargetBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, glm::ivec3 &out);
    
      
      
        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        // std::vector<LveGameObject> gameObjects;
        // std::unordered_map<glm::vec3, LveGameObject> gameObjects;
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> gameObjects;
        Area area;
        glm::ivec4 hoveredID;

        std::shared_ptr<CameraSystem> cameraSystem;
        std::shared_ptr<PhysicsSystem> physicsSystem;
        std::shared_ptr<InputSystem> inputSystem;
        std::shared_ptr<MovementSystem> movementSystem;
        std::shared_ptr<CollisionSystem> collisionSystem;
        std::unique_ptr<ImguiManager> imguiManager;
        
    };
}
