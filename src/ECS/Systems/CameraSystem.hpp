#pragma once

#include "ECS/System.hpp"
#include <set>
#include "ECS/Coordinator.hpp"
#include "ECS/Components/Camera.hpp"
#include <glm/glm.hpp>

namespace lve
{
    class CameraSystem : public System
    {
    public:
        void SetOrthogonalProjection(CameraComponent &camera, float left, float right, float top, float bottom, float near, float far);

        void SetPerspectiveProjection(CameraComponent &camera, float fovy, float aspect, float near, float far);

        void SetViewDirection(CameraComponent &camera, glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

        void SetViewTarget(CameraComponent &camera, glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

        void setViewXYZ(CameraComponent &camera, glm::vec3 position, glm::vec3 rotation);

        void Update(float aspect);
    };
}
