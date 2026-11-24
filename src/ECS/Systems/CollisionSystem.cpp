#include "CollisionSystem.hpp"
#include "World/Area.hpp"
#include "Physics/aabb.hpp"

#include <iostream>
namespace lve
{
    extern Coordinator coordinator;

    void CollisionSystem::Update(float dt, Area &area) {
        for (auto const &entity : mEntities) {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &aabb = coordinator.GetComponent<AABBComponent>(entity);
            auto &rigidBody = coordinator.GetComponent<RigidBodyComponent>(entity);

            float m_smoothedDt = 0.016f;

            // In CollisionSystem::Update(), replace the raw dt usage:
            m_smoothedDt = glm::mix(m_smoothedDt, dt, 0.1f);
            float smoothDt = m_smoothedDt;

            glm::vec3 desiredMove = rigidBody.velocity * dt;
            glm::vec3 actualMove;
            if (aabb.collisionEnabled) {
                actualMove = CollisionDetection::Move(transform, aabb, desiredMove, area);
            } else {
                actualMove = desiredMove;
            }

            transform.position += actualMove;

            rigidBody.isGrounded = false;
            for (int i = 0; i < 3; ++i) {
                if (glm::abs(actualMove[i]) < glm::abs(desiredMove[i])) {
                    if (i == 1 && desiredMove[i] <= 0.f) {
                        rigidBody.isGrounded = true;
                    }
                    rigidBody.velocity[i] = 0.f;
                }
            }
        }
    }
} // namespace lve
