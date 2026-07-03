#include "PhysicsSystem.hpp"
#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MovementStats.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void PhysicsSystem::Init()
    {
    }

    void PhysicsSystem::Update(float dt)
    {
        for (auto const &entity : mEntities)
        {
            auto &rigidBody = coordinator.GetComponent<RigidBodyComponent>(entity);
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &moveStats = coordinator.GetComponent<MovementStats>(entity);

            // Forces
            auto const &gravity = coordinator.GetComponent<GravityComponent>(entity);

            // transform.position += rigidBody.velocity * dt;
            if (!moveStats.flying)
            {
                rigidBody.velocity += gravity.force * dt;
            }
        }
    }
}
