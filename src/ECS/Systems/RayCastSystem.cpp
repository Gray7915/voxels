#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "RayCastSystem.hpp"

#include "ECS/Components/Gravity.hpp"
#include "ECS/Components/RigidBody.hpp"
#include "ECS/Components/Thrust.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/MovementStats.hpp"
#include "ECS/Components/RayCastComponent.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void RayCastSystem::Init()
    {
    }

    void RayCastSystem::Update(float dt)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &rayCast = coordinator.GetComponent<RayCastComponent>(entity);

            p = glm::floor(rayCast.direction);
            step = glm::sign(rayCast.direction);
            t_max = intbound(rayCast.origin, rayCast.direction);
            t_delta = glm::vec3(step) / rayCast.direction;
            radius = 4 / glm::l2Norm(rayCast.direction);
        }
    }

    
}
