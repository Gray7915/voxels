#pragma once

#include "../System.hpp"
#include "../Coordinator.hpp"

namespace lve
{
    class PhysicsSystem : public System
    {
    public:
        void Init();

        void Update(float dt);
    };
}
