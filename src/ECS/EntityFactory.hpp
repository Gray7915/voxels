#pragma once
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Components/MovementStats.hpp"
#include "ECS/Components/Renderable.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/SpawnInfo.hpp"
#include "ECS/Systems/CameraSystem.hpp"

namespace ECS
{
    extern lve::Coordinator coordinator;

    class EntityFactory {
      public:
        static EntityFactory &Get() {
            static EntityFactory instance;
            return instance;
        }

        using Builder = std::function<Entity(const SpawnInfo &)>;

        void Register(const std::string &name, Builder builder) { builders[name] = std::move(builder); }

        Entity Create(const std::string &name, const SpawnInfo &info) { return builders.at(name)(info); }

      private:
        std::unordered_map<std::string, Builder> builders;
    };
} // namespace ECS