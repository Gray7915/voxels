#pragma once

#include "../System.hpp"
#include "../Coordinator.hpp"
#include "../../lve_frame_info.hpp"
#include "../../simple_render_system.hpp"

namespace lve
{
    class RenderSystem : public System
    {
    public:
        void Update(FrameInfo &frameInfo, SimpleRenderSystem &simpleRenderSystem);
    };
};
