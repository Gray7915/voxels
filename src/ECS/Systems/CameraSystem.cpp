#include "CameraSystem.hpp"
#include "../Components/Camera.hpp"
#include "../Components/Transform.hpp"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <cassert>
#include <limits>

namespace lve
{
    extern Coordinator coordinator;

    void CameraSystem::SetPerspectiveProjection(CameraComponent &camera, float fovy, float aspect, float near, float far)
    {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        camera.projectionMatrix = glm::mat4{0.0f};
        camera.projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        camera.projectionMatrix[1][1] = 1.f / tanHalfFovy;
        camera.projectionMatrix[2][2] = far / (far - near);
        camera.projectionMatrix[2][3] = 1.f;
        camera.projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void CameraSystem::SetViewDirection(CameraComponent &camera, glm::vec3 position, glm::vec3 direction, glm::vec3 up)
    {
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};
        camera.viewMatrix = glm::mat4{1.f};
        camera.viewMatrix[0][0] = u.x;
        camera.viewMatrix[0][1] = v.x;
        camera.viewMatrix[0][2] = w.x;
        camera.viewMatrix[1][0] = u.y;
        camera.viewMatrix[1][1] = v.y;
        camera.viewMatrix[1][2] = w.y;
        camera.viewMatrix[2][0] = u.z;
        camera.viewMatrix[2][1] = v.z;
        camera.viewMatrix[2][2] = w.z;
        camera.viewMatrix[3][0] = -glm::dot(u, position);
        camera.viewMatrix[3][1] = -glm::dot(v, position);
        camera.viewMatrix[3][2] = -glm::dot(w, position);
    }

    void CameraSystem::SetViewTarget(CameraComponent &camera, glm::vec3 position, glm::vec3 target, glm::vec3 up)
    {
        SetViewDirection(camera, position, target - position, up);
    }

    void CameraSystem::setViewXYZ(CameraComponent &camera, glm::vec3 position, glm::vec3 rotation)
    {
        const float c3 = glm::cos(rotation.z), s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x), s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y), s1 = glm::sin(rotation.y);

        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};

        camera.viewMatrix = glm::mat4{1.f};
        camera.viewMatrix[0][0] = u.x;
        camera.viewMatrix[0][1] = v.x;
        camera.viewMatrix[0][2] = w.x;
        camera.viewMatrix[1][0] = u.y;
        camera.viewMatrix[1][1] = v.y;
        camera.viewMatrix[1][2] = w.y;
        camera.viewMatrix[2][0] = u.z;
        camera.viewMatrix[2][1] = v.z;
        camera.viewMatrix[2][2] = w.z;
        camera.viewMatrix[3][0] = -glm::dot(u, position);
        camera.viewMatrix[3][1] = -glm::dot(v, position);
        camera.viewMatrix[3][2] = -glm::dot(w, position);
    }

    void CameraSystem::Update(float aspect)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &camera = coordinator.GetComponent<CameraComponent>(entity);

            SetPerspectiveProjection(camera, camera.fovy, aspect, camera.nearPlane, camera.farPlane);
            setViewXYZ(camera, transform.position, transform.rotation);
        }
    }
}
