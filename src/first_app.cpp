#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include "Input/keyboard_movement_controller.hpp"
#include "IVec3Hash.h"
#include "World/cube.hpp"
#include "World/Chunk.hpp"
#include "Util/ray.hpp"
#include "World/ChunkRenderer.hpp"
#include "lve_buffer.hpp"
#include "highlight_render_system.hpp"

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
    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        // glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.02f};
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f}; // w is for light intensity
    };

    FirstApp::FirstApp() : area(gameObjects, lveDevice, glm::vec3(0, 0, 0)), hoveredID(glm::ivec4(0, 0, 0, 0))
    {
        globalPool = LveDescriptorPool::Builder(lveDevice)
                         .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        texture = std::make_unique<LveTexture>(lveDevice, "/home/patrick/Documents/Projects/voxels/Textures/images.jpeg");
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        std::vector<VkDescriptorSet> globaDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globaDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();

            VkDescriptorImageInfo imageInfo{};
            imageInfo.sampler = texture->getSampler();
            imageInfo.imageView = texture->getImageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globaDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        HighlightRenderSystem highlightRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
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

            glm::vec3 rot = viewerObject.transform.rotation;
            glm::vec3 forward = {
                cos(rot.x) * sin(rot.y),
                -sin(rot.x),
                cos(rot.x) * cos(rot.y)};
            glm::vec3 worldUp = {0.f, 1.f, 0.f};

            glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
            glm::vec3 up = glm::cross(forward, right);

            glm::vec3 rayDir = glm::normalize(forward);

            Ray ray(viewerObject.transform.translation, rayDir);
            glm::ivec3 rayHit = ray.detectBlockHit(4.0f); // worldspace
            glm::ivec3 chunkPos = glm::ivec3(viewerObject.transform.translation) / glm::ivec3(16, 32, 16);

            if (rayHit == glm::ivec3(-1.0f))
            {
                hoveredID = glm::ivec4();
            }
            else
            {
                hoveredID = glm::ivec4(rayHit, 1);
            }
            // std::cout << hoveredID.x << " " << hoveredID.y << " " << hoveredID.z << "\n";
            //  area.tick(gameObjects, lveDevice, viewerObject.transform.translation);
            //   start frame and start swapchain pass are not combined to enable multiple render passes
            if (auto commandBuffer = lveRenderer.beginFrame())
            {

                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera, globaDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects, hoveredID);
                highlightRenderSystem.render(frameInfo, hoveredID.w != 0, glm::ivec3(hoveredID));
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
                std::cout << "Ray Hit " << rayHit.x << " " << rayHit.y << " " << rayHit.z << '\n';
                // std::cout << "Ray Orgin " << viewerObject.transform.translation.x << " " << viewerObject.transform.translation.y << " " << viewerObject.transform.translation.z << '\n';
                // std::cout << "Ray Direction " << forward.x << " " << forward.y << " " << forward.z << '\n';

                Area::chunks.find(chunkPos)->second->blocks[rayHit.x % 16][rayHit.y % 32][rayHit.z % 16] = 0;
                // std::cout << "array place found" << '\n';
                // gameObjects.find(chunkPos)->second.model.reset();
                gameObjects.find(chunkPos)->second.model = ChunkRenderer::mesh(Area::chunks.find(chunkPos)->second->blocks, lveDevice, {0, 0, 0}).model;

                // std::cout << "chunk changed" << '\n';

                vkDeviceWaitIdle(lveDevice.device());
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
