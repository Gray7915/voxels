#pragma once

#include "ECS/Coordinator.hpp"

#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/InputSystem.hpp"
#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"

namespace lve
{
    struct ECSSystems
    {
        std::shared_ptr<PhysicsSystem> physicsSystem;
        std::shared_ptr<CameraSystem> cameraSystem;
        std::shared_ptr<InputSystem> inputSystem;
        std::shared_ptr<MovementSystem> movementSystem;
        std::shared_ptr<CollisionSystem> collisionSystem;
    };

    ECSSystems registerECSComponents(Coordinator &coordinator);

}
