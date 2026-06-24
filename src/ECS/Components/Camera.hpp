#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve{
    struct CameraComponent{
        glm::mat4 projectionMatrix{1.f};
        glm::mat4 viewMatrix{1.f};

        float fovy = glm::radians(50.f);
        float nearPlane = 0.1f;
        float farPlane = 100.f;
    };
}
