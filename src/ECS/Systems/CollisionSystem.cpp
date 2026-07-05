#include "CollisionSystem.hpp"
#include "World/Area.hpp"
#include "Physics/aabb.hpp"

#include <iostream>
namespace lve
{
    extern Coordinator coordinator;

    void CollisionSystem::Update(float dt, Area &area)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &aabb = coordinator.GetComponent<AABBComponent>(entity);
            auto &rigidBody = coordinator.GetComponent<RigidBodyComponent>(entity);

            if (aabb.isTrigger)
            {
                transform.position += rigidBody.velocity * dt;
                continue;
            }

            glm::vec3 desiredMove = rigidBody.velocity * dt;
            glm::vec3 actualMove;
            if (aabb.collisionEnabled)
            {
                actualMove = CollisionDetection::Move(transform, aabb, desiredMove, area);
            }
            else
            {
                actualMove = desiredMove;
            }

            transform.position += actualMove;

            rigidBody.isGrounded = false;
            for (int i = 0; i < 3; ++i)
            {
                if (glm::abs(actualMove[i]) < glm::abs(desiredMove[i]) - 0.0001f)
                {
                    if (i == 1 && desiredMove[i] <= 0.f)
                    {
                        rigidBody.isGrounded = true;
                    }
                    rigidBody.velocity[i] = 0.f;
                }
            }
        }
    }
}
