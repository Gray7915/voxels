#pragma once
#include "../System.hpp"
#include "../Coordinator.hpp"
#include "../Components/Transform.hpp"
#include "../Components/AABBComponent.hpp"
#include "../Components/RigidBody.hpp"

namespace lve
{
    class CollisionSystem : public System
    {
    public:
        void Update(float dt);
        bool CheckTerrainOverlap(const Transform &transform, const AABBComponent &aabbComponent, const RigidBodyComponent &ridgidBody);
        void ResolveAxis(Transform transform, RigidBodyComponent rigidBody, AABBComponent aabb, int axis);
    };
}
