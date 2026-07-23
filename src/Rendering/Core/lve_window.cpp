#include "lve_window.hpp"
#include "ECS/Systems/InputSystem.hpp"
#include <stdexcept>
namespace lve
{
    LveWindow::LveWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
    {
        initWindow();
    }

    LveWindow::~LveWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // GLFW_NO_API stops it from trying to run open gl
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // stops the window from being resized.

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
        glfwSetCursorPosCallback(window, InputSystem::mouse_callback);

        if (glfwRawMouseMotionSupported())
        {
            // Hide and lock the cursor to the window (required for raw motion)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Enable raw mouse motion
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void LveWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        lveWindow->frameBufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }

    void LveWindow::setMouseActive()
    {
        if (glfwRawMouseMotionSupported())
        {
            int currentMode = glfwGetInputMode(window, GLFW_CURSOR);

            if (currentMode == GLFW_CURSOR_DISABLED)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
                glfwSetCursorPos(window, width / 2, height / 2);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
    }

    bool LveWindow::getMenuActive(){
        if(glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED){
            return false;
        }else{
            return true;
        }
    }
}
