#pragma once
#include "ECS/System.hpp"
#include "Rendering/Core/lve_window.hpp"
#include "ECS/Coordinator.hpp"

namespace lve
{
    class InputSystem : public System
    {
    public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_SHIFT;

            int releaseSpace = GLFW_KEY_SPACE;
        };

        void Update(LveWindow *window);

        static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

        static InputSystem *instance;

    private:
        bool firstMouse = true;
        double lastX = 0.0;
        double lastY = 0.0;
        float pendingMouseDeltaX = 0.f;
        float pendingMouseDeltaY = 0.f;
    };
}
