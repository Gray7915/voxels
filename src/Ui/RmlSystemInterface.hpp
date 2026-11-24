#pragma once
#include <RmlUi/Core/SystemInterface.h>
#include <GLFW/glfw3.h>

class RmlSystemInterface : public Rml::SystemInterface {
  public:
    double GetElapsedTime() override { return glfwGetTime(); }
};