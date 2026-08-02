#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_renderer.hpp"
#include "Util/lve_frame_info.hpp"

namespace Rendering
{
    class Scene {
      public:
        virtual ~Scene() = default;
        virtual void onEnter() {}
        virtual void onExit() {}
        virtual void update(float deltaTime) = 0;
        virtual void render(lve::FrameInfo &frameInfo) = 0;
    };
} // namespace Rendering