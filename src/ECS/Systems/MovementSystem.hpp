#pragma once
#include "../System.hpp"
#include "../Coordinator.hpp"

namespace lve
{
    class MovementSystem : public System
    {
    public:
        void Update(float deltaTime);
    };
}
