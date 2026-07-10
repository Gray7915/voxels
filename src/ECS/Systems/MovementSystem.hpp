#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include <chrono>
namespace lve
{
    class MovementSystem : public System
    {
    public:
        void Update(float deltaTime);
        bool releaseSpace = false;
        std::chrono::steady_clock::time_point lastJumpPress{};
    };
}
