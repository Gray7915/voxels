#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "Util/Types.hpp"
#include <chrono>
namespace lve
{
    class MovementSystem : public System {
      public:
        void Update(float deltaTime, u64 frameIndex);
        bool releaseSpace = false;
        std::chrono::steady_clock::time_point lastJumpPress{};
        float m_smoothedDt = 0.016f;
    };
} // namespace lve
