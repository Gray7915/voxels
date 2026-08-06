#include "lve_window.hpp"
#include "ECS/Systems/InputSystem.hpp"
#include <stdexcept>
#include <RmlUi_Platform_GLFW.h>

namespace lve
{
    LveWindow::LveWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} { initWindow(); }

    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // GLFW_NO_API stops it from trying to run open gl
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // stops the window from being resized.

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();

        int wx, wy, ww, wh;
        glfwGetMonitorWorkarea(monitor, &wx, &wy, &ww, &wh);

        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        int x = wx + (ww - windowWidth) / 2;
        int y = wy + (wh - windowHeight) / 2;

        glfwSetWindowPos(window, x, y);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCharCallback(window, charCallback);

        if (glfwRawMouseMotionSupported()) {
            // Hide and lock the cursor to the window (required for raw motion)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Enable raw mouse motion
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void LveWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        lveWindow->frameBufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }

    void LveWindow::setMouseActive() {
        if (glfwRawMouseMotionSupported()) {
            int currentMode = glfwGetInputMode(window, GLFW_CURSOR);

            if (currentMode == GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
                glfwSetCursorPos(window, width / 2, height / 2);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
    }

    bool LveWindow::getMenuActive() {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            return false;
        } else {
            return true;
        }
    }

    void LveWindow::setRmlContext(Rml::Context *context) { rmlContext = context; }

    void LveWindow::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
        auto lveWindow = static_cast<LveWindow *>(glfwGetWindowUserPointer(window));

        // Game camera
        InputSystem::mouse_callback(window, xpos, ypos);

        // UI
        if (lveWindow->rmlContext) {
            lveWindow->rmlContext->ProcessMouseMove(static_cast<int>(xpos), static_cast<int>(ypos), 0);
        }
    }

    void LveWindow::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        auto lveWindow = static_cast<LveWindow *>(glfwGetWindowUserPointer(window));

        if (!lveWindow->rmlContext)
            return;

        if (action == GLFW_PRESS)
            lveWindow->rmlContext->ProcessMouseButtonDown(button, mods);

        if (action == GLFW_RELEASE)
            lveWindow->rmlContext->ProcessMouseButtonUp(button, mods);
    }

    void LveWindow::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto lveWindow = static_cast<LveWindow *>(glfwGetWindowUserPointer(window));

        // Always send to your game input system

        // Also send to RmlUi if it exists
        if (lveWindow->rmlContext) {
            auto rmlKey = RmlGLFW::ConvertKey(key);

            if (action == GLFW_PRESS || action == GLFW_REPEAT)
                lveWindow->rmlContext->ProcessKeyDown(rmlKey, mods);

            if (action == GLFW_RELEASE)
                lveWindow->rmlContext->ProcessKeyUp(rmlKey, mods);
        }
    }

    void LveWindow::charCallback(GLFWwindow *window, unsigned int codepoint) {
        auto lveWindow = static_cast<LveWindow *>(glfwGetWindowUserPointer(window));

        if (lveWindow->rmlContext) {
            lveWindow->rmlContext->ProcessTextInput(static_cast<Rml::Character>(codepoint));
        }
    }
} // namespace lve
