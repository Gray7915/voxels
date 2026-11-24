#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "ECS/Components/InteractionProgress.hpp"

#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Core/lve_device.hpp"

#include "World/Area.hpp"

namespace lve
{
    class InteractionSystem : public System {
      public:
        void Update(float deltaTime, LveWindow &lveWindow, LveDevice &lveDevice, Area &area, Coordinator &coordinator);
        ivec4 hoveredID{0};

      private:
        void expireTargets(InteractionProgress &, float dt);
        void updateMining(Entity entity, float dinteractionProgresst, Area &area, Coordinator &coord, InteractionProgress &progress, const RayHit &hit);
        void updatePlacing(Entity entity, float dt, Area &area, Coordinator &coord, const RayHit &hit, InventoryComponent &inv, const InputComponent &input);

        InteractionTarget *findTarget(InteractionProgress &interactionProgress, ivec3 pos);
        InteractionTarget &beginTarget(InteractionProgress &interactionProgress, const RayHit &);
        double breakTimeFor(const Block &) const;
    };
} // namespace lve
