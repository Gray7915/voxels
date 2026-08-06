#pragma once

#include "ECS/Coordinator.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/SpawnInfo.hpp"

#include "Util/Types.hpp"

namespace lve
{
    class EntityFactorySetup {
      public:
        static void SetupEntityFactory(ECS::EntityFactory &factory, Coordinator &coordinator) {
            factory.Register("MainCamera", [&](const ECS::SpawnInfo &info) {
                Entity entity = coordinator.CreateEntity();

                coordinator.AddComponent(entity, GravityComponent{vec3(0, -15, 0)});
                coordinator.AddComponent(entity, RigidBodyComponent{});
                coordinator.AddComponent(entity, InputComponent{});
                coordinator.AddComponent(entity, MovementStats{6.5f, 6.1f});
                coordinator.AddComponent(entity, AABBComponent{.halfExtents = vec3(0.4f, 0.8f, 0.4f)});
                coordinator.AddComponent(entity, InventoryComponent{});
                coordinator.AddComponent(entity, Transform{.position = info.position});
                coordinator.AddComponent(entity, CameraComponent{});

                return entity;
            });
        }
    };
} // namespace lve
