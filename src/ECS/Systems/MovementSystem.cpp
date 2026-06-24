#include "MovementSystem.hpp"
#include "../Components/Transform.hpp"
#include "../Components/Input.hpp"
#include "ECS/Components/MovementStats.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve
{
    extern Coordinator coordinator;

    void MovementSystem::Update(float dt)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &input = coordinator.GetComponent<InputComponent>(entity);
            auto &moveStats = coordinator.GetComponent<MovementStats>(entity);

            transform.rotation.y += input.mouseDeltaX * moveStats.mouseSensitivity;
            transform.rotation.x -= input.mouseDeltaY * moveStats.mouseSensitivity;
            transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
            transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

            float yaw = transform.rotation.y;
            const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
            const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
            const glm::vec3 upDir{0.f, -1.f, 0.f};

            glm::vec3 moveDir{0.f};
            if (input.moveForward)
                moveDir += forwardDir;
            if (input.moveBackward)
                moveDir -= forwardDir;
            if (input.moveRight)
                moveDir += rightDir;
            if (input.moveLeft)
                moveDir -= rightDir;
            if (input.moveUp)
                moveDir += upDir;
            if (input.moveDown)
                moveDir -= upDir;

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            {
                transform.position += moveStats.moveSpeed * dt * glm::normalize(moveDir);
            }
        }
    }

}
