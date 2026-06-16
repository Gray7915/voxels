#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include "Input/keyboard_movement_controller.hpp"
#include "IVec3Hash.h"
#include "World/cube.hpp"
#include "World/Chunk.hpp"
#include "Util/ray.hpp"

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

namespace lve
{

    FirstApp::FirstApp() : area(gameObjects, lveDevice, glm::vec3(0, 0, 0))
    {
        // loadGameObjects();
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
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            // std::cout << "camera coordinate " << " " << viewerObject.transform.translation.x << " " << viewerObject.transform.translation.y << " " << viewerObject.transform.translation.z << '\n';

            area.tick(gameObjects, lveDevice, viewerObject.transform.translation);
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
            if (pWasPressed && !pIsPressed)
                pWasPressed = false;
            if (pIsPressed && !pWasPressed)
            {
                pWasPressed = true;
                glm::vec3 rot = viewerObject.transform.rotation;
                glm::vec3 thingie = {glm::cos(rot.x) * glm::sin(rot.y), glm::cos(rot.x), glm::cos(rot.x) * glm::sin(rot.y)};
                glm::vec3 rayDir = glm::normalize(thingie);

                Ray ray(glm::ivec3(viewerObject.transform.translation), rayDir);
                glm::ivec3 rayHit = ray.detectBlockHit(4.0f);
                glm::vec3 chunkPos = glm::ivec3( viewerObject.transform.translation) / glm::ivec3(16,32,16);
                Area::chunks.find(chunkPos)->second->blocks[rayHit.x][rayHit.y][rayHit.z] = 0;
                std::cout << "Ray Hit " << rayHit.x << " " << rayHit.y << " " << rayHit.z << '\n';
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
