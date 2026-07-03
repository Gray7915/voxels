#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"

namespace lve
{
    class MovementSystem : public System
    {
    public:
        void Update(float deltaTime);
        bool releaseSpace = false;
    };
}
