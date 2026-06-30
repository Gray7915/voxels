#pragma once
#include "../System.hpp"
#include "../Coordinator.hpp"
#include "../../lve_window.hpp"
#include "../../lve_device.hpp"

#include "../Components/Transform.hpp"
#include "../Components/AABBComponent.hpp"
#include "../Components/Camera.hpp"

namespace lve
{
    class InteractionSystem : public System
    {
    public:
        void Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice);
        bool CheckBlockPlacement(const Transform &transform, const AABBComponent &aabbComponent, glm::ivec3 position);
        glm::ivec4 hoveredID{0};
    };
}
