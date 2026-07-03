#include "SetupECS.hpp"

#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/MovementStats.hpp"
#include "ECS/Components/ColliderComponent.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Renderable.hpp"

namespace lve
{
    ECSSystems registerECSComponents(Coordinator &coordinator)
    {
        ECSSystems systems;

        // --- Component registration ---
        coordinator.RegisterComponent<GravityComponent>();
        coordinator.RegisterComponent<RigidBodyComponent>();
        coordinator.RegisterComponent<ThrustComponent>();
        coordinator.RegisterComponent<Transform>();
        coordinator.RegisterComponent<CameraComponent>();
        coordinator.RegisterComponent<InputComponent>();
        coordinator.RegisterComponent<MovementStats>();
        coordinator.RegisterComponent<AABBComponent>();
        coordinator.RegisterComponent<RenderableComponent>();

        // --- PhysicsSystem ---
        systems.physicsSystem = coordinator.RegisterSystem<PhysicsSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<GravityComponent>());
            signature.set(coordinator.GetComponentType<RigidBodyComponent>());
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<MovementStats>());
            coordinator.SetSystemSignature<PhysicsSystem>(signature);
        }

        // --- CameraSystem ---
        systems.cameraSystem = coordinator.RegisterSystem<CameraSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<CameraComponent>());
            coordinator.SetSystemSignature<CameraSystem>(signature);
        }

        // --- InputSystem ---
        systems.inputSystem = coordinator.RegisterSystem<InputSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<InputComponent>());
            coordinator.SetSystemSignature<InputSystem>(signature);

            // NOTE: this static/singleton assignment was present in the
            // original FirstApp::registerECSComponents(). Kept as-is during
            // the split; worth revisiting separately since it's global
            // mutable state tied to setup order.
            InputSystem::instance = systems.inputSystem.get();
        }

        // --- MovementSystem ---
        systems.movementSystem = coordinator.RegisterSystem<MovementSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<InputComponent>());
            signature.set(coordinator.GetComponentType<MovementStats>());
            signature.set(coordinator.GetComponentType<RigidBodyComponent>());
            coordinator.SetSystemSignature<MovementSystem>(signature);
        }

        // --- CollisionSystem ---
        systems.collisionSystem = coordinator.RegisterSystem<CollisionSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<AABBComponent>());
            signature.set(coordinator.GetComponentType<RigidBodyComponent>());
            coordinator.SetSystemSignature<CollisionSystem>(signature);
        }

        systems.interactionSystem = coordinator.RegisterSystem<InteractionSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<AABBComponent>());
            signature.set(coordinator.GetComponentType<CameraComponent>());
            coordinator.SetSystemSignature<InteractionSystem>(signature);
        }

        systems.renderSystem = coordinator.RegisterSystem<RenderSystem>();
        {
            Signature signature;
            signature.set(coordinator.GetComponentType<Transform>());
            signature.set(coordinator.GetComponentType<RenderableComponent>());
            coordinator.SetSystemSignature<RenderSystem>(signature);
        }

        coordinator.eventBus = EventBus{};
        return systems;
    }

}
