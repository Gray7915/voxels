#include "InputSystem.hpp"
#include "../Components/Input.hpp"

namespace lve
{
    extern Coordinator coordinator;

    InputSystem *InputSystem::instance = nullptr;

    void InputSystem::Update(GLFWwindow *window)
    {
        for (auto const &entity : mEntities)
        {
            auto &input = coordinator.GetComponent<InputComponent>(entity);

            input.moveForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
            input.moveBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
            input.moveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
            input.moveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
            input.jump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            input.moveUp = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            input.moveDown = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
            input.mouseDeltaX = pendingMouseDeltaX;
            input.mouseDeltaY = pendingMouseDeltaY;
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
