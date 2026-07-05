#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/RigidBody.hpp"

#include "World/Area.hpp"

namespace lve
{
    class CollisionSystem : public System
    {
    public:
        void Update(float dt, Area &area);
    };
}
