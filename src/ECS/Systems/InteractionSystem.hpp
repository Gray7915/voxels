#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Camera.hpp"

#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Core/lve_device.hpp"


#include "World/Area.hpp"

namespace lve
{
    class InteractionSystem : public System
    {
    public:
        void Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice, Area &area);
        bool CheckBlockPlacement(const Transform &transform, const AABBComponent &aabbComponent, glm::ivec3 position);
        glm::ivec4 hoveredID{0};
    };
}
