#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "IVec3Hash.h"
#include "ChunkSystem/cube.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace lve
{

    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveCamera camera{};

        // camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = LveGameObject::createGameObject();
        KeyboardMovementController cameraController{};
        KeyboardMovementController::instance = &cameraController;

        auto currentTime = std::chrono::high_resolution_clock::now();

        viewerObject.transform.translation.y = -4;
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ((viewerObject.transform.translation), viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            // start frame and start swapchain pass are not combined to enable multiple render passes
            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

            static bool pWasPressed = false;
            bool pIsPressed = glfwGetMouseButton(lveWindow.getGLFWwindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;

            glm::vec3 rot = viewerObject.transform.rotation;
            glm::vec3 rayDir = {glm::sin(rot.y) * glm::cos(rot.x), -glm::sin(rot.x), glm::cos(rot.y) * glm::cos(rot.x)};

            glm::ivec3 *target = getTargetBlock(viewerObject.transform.translation, rayDir, gameObjects);
            if (target != nullptr && (pIsPressed && !pWasPressed))
            {
                vkDeviceWaitIdle(lveDevice.device());
                gameObjects.find(*target)->second.model.reset();
                gameObjects.erase(*target);
                pWasPressed = pIsPressed;
            }
            else
            {
                vkDeviceWaitIdle(lveDevice.device());
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    };

    void FirstApp::loadGameObjects()
    {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/colored_cube.obj");

        auto cube = LveGameObject::createGameObject();
        cube.model = lveModel;
        cube.transform.translation = {.0f, .0f, .0f};
        cube.transform.scale = {1.f, 1.f, 1.f};
        gameObjects.insert({cube.transform.translation, std::move(cube)});

        auto cube2 = LveGameObject::createGameObject();
        cube2.model = lveModel;
        cube2.transform.translation = {1.0f, .0f, .0f};
        cube2.transform.scale = {1.f, 1.f, 1.f};
        gameObjects.insert({cube2.transform.translation, std::move(cube2)});

        auto cube3 = LveGameObject::createGameObject();
        cube3.model = lveModel;
        cube3.transform.translation = {2.0f, .0f, .0f};
        cube3.transform.scale = {1.f, 1.f, 1.f};
        gameObjects.insert({cube3.transform.translation, std::move(cube3)});

        auto cube4 = LveGameObject::createGameObject();
        cube4.model = lveModel;
        cube4.transform.translation = {3.0f, .0f, .0f};
        cube4.transform.scale = {1.f, 1.f, 1.f};
        gameObjects.insert({cube4.transform.translation, std::move(cube4)});
    }

    glm::ivec3 *FirstApp::getTargetBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects)
    {
        const float REACH = 5.0f;
        const float STEP = 0.1f;

        glm::vec3 rayDir = glm::normalize(rayDirection);
        float dist = 0.0f;

        while (dist < REACH)
        {
            glm::vec3 point = rayOrigin + rayDir * dist;
            glm::ivec3 coord = glm::ivec3(glm::floor(point));

            auto it = gameObjects.find(coord);
            if (it != gameObjects.end())
            {
                return const_cast<glm::ivec3 *>(&it->first);
            }

            dist += STEP;
        }
        return nullptr;
    }
}
