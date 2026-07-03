#pragma once
#include "ECS/System.hpp"
#include "ECS/Coordinator.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/RigidBody.hpp"

namespace lve
{
    class CollisionSystem : public System
    {
    public:
        void Update(float dt);
        bool CheckTerrainOverlap(const Transform &transform, const AABBComponent &aabbComponent); 
        bool CheckBlockPlacement(const Transform &transform, const AABBComponent &aabbComponent, glm::ivec3 position);
        void ResolveAxis(Transform transform, RigidBodyComponent rigidBody, AABBComponent aabb, int axis);
        float MoveAxis(const Transform &transform, const AABBComponent &aabb, float movement, int axis);
        glm::vec3 Move(const Transform &transform, const AABBComponent &aabb, glm::vec3 movement);
    };
}
