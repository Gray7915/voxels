#pragma once

#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "Util/lve_frame_info.hpp"
#include "Rendering/Systems/simple_render_system.hpp"

namespace lve
{
    class RenderSystem : public System
    {
    public:
        void Update(FrameInfo &frameInfo, SimpleRenderSystem &simpleRenderSystem);
    };
};
