#include "ECS/Systems/InputSystem.hpp"
#include "ECS/Components/Input.hpp"
#include <iostream>

namespace lve
{
    extern Coordinator coordinator;

    InputSystem *InputSystem::instance = nullptr;

    void InputSystem::Update(LveWindow *window)
    {
        for (auto const &entity : mEntities)
        {
            auto &input = coordinator.GetComponent<InputComponent>(entity);

            input.moveForward = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS;
            input.moveBackward = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS;
            input.moveLeft = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS;
            input.moveRight = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS;
            input.jump = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS;
            input.moveUp = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS;
            input.moveDown = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

            input.releaseSpace = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_RELEASE;

            input.escape = glfwGetKey(window->getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS;

            static constexpr std::array<int, 10> hotbarKeys =
                {GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
                 GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0};

            for (int i = 0; i < hotbarKeys.size(); i++)
            {
                if (glfwGetKey(window->getGLFWwindow(), hotbarKeys[i]) == GLFW_PRESS)
                {
                    input.hotbarSlot = i;
                }
            }

            input.mouseDeltaX = pendingMouseDeltaX;
            input.mouseDeltaY = pendingMouseDeltaY;

            input.leftMouseClicked = glfwGetMouseButton(window->getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            input.middleMouseClicked = glfwGetMouseButton(window->getGLFWwindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
            input.rightMouseClicked = glfwGetMouseButton(window->getGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        }

        pendingMouseDeltaX = 0.f;
        pendingMouseDeltaY = 0.f;
    }

    void InputSystem::mouse_callback(GLFWwindow *window, double xpos, double ypos)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            if (instance->firstMouse)
            {
                instance->lastX = xpos;
                instance->lastY = ypos;
                instance->firstMouse = false;
                return;
            }

            instance->pendingMouseDeltaX = static_cast<float>(xpos - instance->lastX);
            instance->pendingMouseDeltaY = static_cast<float>(ypos - instance->lastY);
            instance->lastX = xpos;
            instance->lastY = ypos;
        }
        else
        {
            instance->firstMouse = true;
        }
    }
}
