#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include <unordered_map>
#include "IVec3Hash.h"

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
        glm::ivec3 *getTargetBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects);
        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        // std::vector<LveGameObject> gameObjects;
        // std::unordered_map<glm::vec3, LveGameObject> gameObjects;
        std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> gameObjects;
    };
}
