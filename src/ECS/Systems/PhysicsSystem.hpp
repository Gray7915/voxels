#pragma once

#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"

namespace lve
{
    class PhysicsSystem : public System
    {
    public:
        void Init();

        void Update(float dt);
    };
}
