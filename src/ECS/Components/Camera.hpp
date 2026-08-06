#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Util/Frustrum.hpp"

struct CameraComponent
{
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};
    glm::vec3 relativePosition{0, 0.8f, 0}; // position of camera relative to player. 0.0f means the camera sits exactly at the players location rather than at eye level.

    Frustum frustum;

    float fovy = glm::radians(50.f);
    float nearPlane = 0.1f;
    float farPlane = 500.f;
};
