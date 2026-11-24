#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <RmlUi/Core.h>

namespace lve
{
    class LveWindow {

      public:
        LveWindow(int w, int h, std::string name);
        ~LveWindow();

        LveWindow(const LveWindow &) = delete;
        LveWindow &operator=(const LveWindow &) = delete;
        void setRmlContext(Rml::Context *context);

        bool shouldClose() { return glfwWindowShouldClose(window); }

        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        GLFWwindow *getGLFWwindow() const { return window; };

        void setMouseActive();

        static void mouseThingie();
        bool getMenuActive();

      private:
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;
        GLFWwindow *window;

        Rml::Context *rmlContext = nullptr;

        static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void charCallback(GLFWwindow *window, unsigned int codepoint);
    };
} // namespace lve
