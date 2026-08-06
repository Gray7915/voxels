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
#include <fstream>
#include <GLFW/glfw3.h>

namespace lve
{
    extern Coordinator coordinator;

    // CSV log file — opened once, stays open for the lifetime of the program.
    static std::ofstream s_velocityLog;

    // Call once before the first Update() to open the file and write the header.
    static void EnsureVelocityLogOpen() {
        if (s_velocityLog.is_open())
            return;

        s_velocityLog.open("velocity_log.csv", std::ios::out | std::ios::trunc);
        if (!s_velocityLog.is_open()) {
            std::cerr << "[MovementSystem] WARNING: could not open velocity_log.csv for writing.\n";
            return;
        }

        // CSV header
        s_velocityLog << "deltaTime, smoothDeltaTime ,frame,vx,vy,vz,speed_xz,speed_3d,any_input,forward,backward,left,right\n";
    }

    void MovementSystem::Update(float dt, u64 frameIndex) {

        // At the top of Update(), replace raw dt usage with smoothed version
        m_smoothedDt = glm::clamp(dt, 0.0f, 0.0333f);
        float smoothDt = m_smoothedDt;

        EnsureVelocityLogOpen();

        for (auto const &entity : mEntities) {
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
                rigidBody.velocity.y *= pow(friction, smoothDt * 60.0f);

            float frictionFactor = pow(friction, smoothDt * 60.0f);
            rigidBody.velocity.x *= frictionFactor;
            rigidBody.velocity.z *= frictionFactor;

            float accel = rigidBody.isGrounded ? moveStats.groundAcceleration : moveStats.airAcceleration;
            if (moveStats.flying) {
                accel = moveStats.groundAcceleration;
                rigidBody.velocity.y += moveDir.y * accel * smoothDt;
            }

            rigidBody.velocity.x += moveDir.x * accel * smoothDt;
            rigidBody.velocity.z += moveDir.z * accel * smoothDt;

            if (moveStats.flying) {
                if (input.moveUp)
                    rigidBody.velocity.y = moveStats.moveSpeed;
                else if (input.moveDown)
                    rigidBody.velocity.y = -moveStats.moveSpeed;
            }

            if (input.jump && !releaseSpace) {
                auto now = std::chrono::steady_clock::now();

                float elapsed = std::chrono::duration<float>(now - lastJumpPress).count();

                if (elapsed <= 0.25f) {
                    moveStats.flying = !moveStats.flying;
                }

                lastJumpPress = now;

                if (rigidBody.isGrounded) {
                    rigidBody.velocity.y = moveStats.jumpForce;
                }
            }

            releaseSpace = input.jump;

            // ── Per-frame velocity CSV logging ──────────────────────────────
            if (s_velocityLog.is_open()) {
                const glm::vec3 &v = rigidBody.velocity;
                float speedXZ = glm::length(glm::vec2(v.x, v.z));
                float speed3D = glm::length(v);

                // 1 if any directional key is held, 0 if no input
                int anyInput = (input.moveForward || input.moveBackward || input.moveLeft || input.moveRight) ? 1 : 0;

                s_velocityLog << dt << ',' << smoothDt << ',' << frameIndex << ',' << v.x << ',' << v.y << ',' << v.z << ',' << speedXZ << ',' << speed3D << ',' << anyInput << ',' << input.moveForward
                              << ',' << input.moveBackward << ',' << input.moveLeft << ',' << input.moveRight << '\n';
            }
            // ────────────────────────────────────────────────────────────────
        }
    }
} // namespace lve