#include "PhysicsSystem.hpp"
#include "../Components/Gravity.hpp"
#include "../Components/RigidBody.hpp"
#include "../Components/Thrust.hpp"
#include "../Components/Transform.hpp"

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

            // Forces
            auto const &gravity = coordinator.GetComponent<GravityComponent>(entity);

            transform.position += rigidBody.velocity * dt;

            rigidBody.velocity += gravity.force * dt;
        }
    }
}
