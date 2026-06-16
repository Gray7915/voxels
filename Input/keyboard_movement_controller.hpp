#pragma once

#include "../lve_game_object.hpp"
#include "../lve_window.hpp"

namespace lve
{
    class KeyboardMovementController
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
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow *window, float dt, LveGameObject &gameObject);

        KeyMappings keys{};
        float moveSpeed{18.f};
        float lookSpeed{1.5f};
        float mouseSensitivity{0.02f};

        static KeyboardMovementController *instance;
        static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

    private:
        double lastX = 0.0, lastY = 0.0;
        float mouseDeltaX = 0.0f, mouseDeltaY = 0.0f;
        bool firstMouse = true;
    };

}
