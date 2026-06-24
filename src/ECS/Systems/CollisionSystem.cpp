#include "CollisionSystem.hpp"

#include "World/Area.hpp"
namespace lve
{
    extern Coordinator coordinator;

    void CollisionSystem::Update(float dt)
    {
        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &aabb = coordinator.GetComponent<AABBComponent>(entity);
            auto &rigidBody = coordinator.GetComponent<RigidBodyComponent>(entity);

            aabb.isTrigger = CheckTerrainOverlap(transform, aabb, rigidBody);
        }
    }

    bool CollisionSystem::CheckTerrainOverlap(const Transform &transform, const AABBComponent &aabbComponent, const RigidBodyComponent &rigidBody)
    {
        glm::vec3 minPos = transform.position - aabbComponent.halfExtents;
        glm::vec3 maxPos = transform.position + aabbComponent.halfExtents;

        glm::ivec3 minBlock = glm::floor(minPos);
        glm::ivec3 maxBlock = glm::floor(maxPos);

        ResolveAxis(transform, rigidBody, aabbComponent, 0); // X
        ResolveAxis(transform, rigidBody, aabbComponent, 1); // Y (gravity/ground)
        ResolveAxis(transform, rigidBody, aabbComponent, 2);

        for (int x = minBlock.x; x <= maxBlock.x; ++x)
            for (int y = minBlock.y; y <= maxBlock.y; ++y)
                for (int z = minBlock.z; z <= maxBlock.z; ++z)
                {
                    if (lve::Area::isBlockSolid(glm::ivec3(x, y, z)))
                        return true;
                }

        return false;
    }

    void CollisionSystem::ResolveAxis(Transform transform, RigidBodyComponent rigidBody, AABBComponent aabb, int axis)
    {
        float originalPos = transform.position[axis];
        transform.position[axis] += rigidBody.velocity[axis];

        if (Area::IsBlockSolid(glm::vec3))
        {
            // collision on this axis only — revert this axis, zero this axis's velocity
            transform.position[axis] = originalPos;
            rigidBody.velocity[axis] = 0.f;
        }
    }
}
