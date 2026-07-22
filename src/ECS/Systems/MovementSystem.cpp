#include "MovementSystem.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/MovementStats.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <optional>
#include <GLFW/glfw3.h>

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
            auto &rigidBody = coordinator.GetComponent<RigidBodyComponent>(entity);

            // Camera rotation
            transform.rotation.y += input.mouseDeltaX * moveStats.mouseSensitivity;
            transform.rotation.x += input.mouseDeltaY * moveStats.mouseSensitivity;
            transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
            transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

            // Movement basis
            float yaw = transform.rotation.y;

            const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
            const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
            const glm::vec3 upDir{0.f, 1.f, 0.f};

            // Build movement direction
            glm::vec3 moveDir(0.0f);

            if (input.moveForward)
                moveDir += forwardDir;
            if (input.moveBackward)
                moveDir -= forwardDir;
            if (input.moveRight)
                moveDir += rightDir;
            if (input.moveLeft)
                moveDir -= rightDir;

            if (glm::length(moveDir) > 0.0f)
                moveDir = glm::normalize(moveDir);

            float friction = rigidBody.isGrounded ? moveStats.groundFriction : moveStats.airFriction;
            if (moveStats.flying)
                rigidBody.velocity.y *= friction * dt;

            rigidBody.velocity.x *= friction;
            rigidBody.velocity.z *= friction;

            float accel = rigidBody.isGrounded ? moveStats.groundAcceleration : moveStats.airAcceleration;
            if (moveStats.flying)
            {
                accel = moveStats.groundAcceleration;
                rigidBody.velocity.y += moveDir.y * accel * dt;
            }

            rigidBody.velocity.x += moveDir.x * accel * dt;
            rigidBody.velocity.z += moveDir.z * accel * dt;

            // std::cout << "acceleration " << accel << '\n';

            // rigidBody.velocity.x = rigidBody.velocity.x;

            if (moveStats.flying)
            {
                if (input.moveUp)
                    rigidBody.velocity.y = moveStats.moveSpeed;
                else if (input.moveDown)
                    rigidBody.velocity.y = -moveStats.moveSpeed;
            }

            if (input.jump && !releaseSpace)
            {
                auto now = std::chrono::steady_clock::now();

                float elapsed = std::chrono::duration<float>(now - lastJumpPress).count();

                if (elapsed <= 0.25f)
                {
                    moveStats.flying = !moveStats.flying;
                    // std::cout << "is flying? " << moveStats.flying << '\n';
                }

                lastJumpPress = now;

                if (rigidBody.isGrounded)
                {
                    rigidBody.velocity.y = moveStats.jumpForce;
                    // std::cout << "is jumping? " << rigidBody.velocity.y << '\n';
                }
            }

            releaseSpace = input.jump;
        }
    }
}
