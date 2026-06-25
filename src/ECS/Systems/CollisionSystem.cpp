#include "CollisionSystem.hpp"

#include "World/Area.hpp"
#include <iostream>
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

            if (aabb.isTrigger)
            {
                transform.position += rigidBody.velocity * dt;
                continue;
            }

            glm::vec3 desiredMove = rigidBody.velocity * dt;
            glm::vec3 actualMove = Move(transform, aabb, desiredMove);

            transform.position += actualMove;

            rigidBody.isGrounded = false;
            for (int i = 0; i < 3; ++i)
            {
                if (glm::abs(actualMove[i]) < glm::abs(desiredMove[i]) - 0.0001f)
                {
                    if (i == 1 && desiredMove[i] <= 0.f)
                    {
                        rigidBody.isGrounded = true;
                    }
                    rigidBody.velocity[i] = 0.f;
                }
            }
        }
    }

    bool CollisionSystem::CheckTerrainOverlap(const Transform &transform, const AABBComponent &aabbComponent)
    {
        glm::vec3 minPos = transform.position - aabbComponent.halfExtents;
        glm::vec3 maxPos = transform.position + aabbComponent.halfExtents;

        glm::ivec3 minBlock = glm::floor(minPos);
        glm::ivec3 maxBlock = glm::floor(maxPos);
        // std::cout << "halfExtents: " << aabbComponent.halfExtents.x << "," << aabbComponent.halfExtents.y << "," << aabbComponent.halfExtents.z << "  minBlock: " << minBlock.x << "," << minBlock.y << "," << minBlock.z << "  maxBlock: " << maxBlock.x << "," << maxBlock.y << "," << maxBlock.z << '\n';
        for (int x = minBlock.x; x <= maxBlock.x; ++x)
            for (int y = minBlock.y; y <= maxBlock.y; ++y)
                for (int z = minBlock.z; z <= maxBlock.z; ++z)
                {
                    if (lve::Area::isBlockSolid(glm::ivec3(x, y, z)))
                    {
                        // std::cout << "  block " << x << "," << y << "," << z << " solid= true" << '\n';
                        return true;
                    }
                }
        return false;
    }

    float CollisionSystem::MoveAxis(const Transform &transform, const AABBComponent &aabb, float movement, int axis)
    {
        constexpr float EPSILON = 0.05f;

        glm::vec3 testPos = transform.position;
        testPos[axis] += movement;

        Transform testTransform = transform;
        testTransform.position = testPos;

        if (!CheckTerrainOverlap(testTransform, aabb))
        {
            return movement;
        }

        float sign = glm::sign(movement);
        float lo = 0.f;
        float hi = movement;

        // loop 6 times for some precision on collision
        // move forward or backward depending on if its a collision or not
        for (int i = 0; i < 6; ++i)
        {
            float mid = (lo + hi) / 2.f;
            testPos = transform.position;
            testPos[axis] += mid;
            testTransform.position = testPos;

            if (CheckTerrainOverlap(testTransform, aabb))
                hi = mid;
            else
                lo = mid;
        }

        float allowed = lo - sign * EPSILON;
        return (glm::abs(allowed) <= EPSILON) ? 0.f : allowed;
    }

    glm::vec3 CollisionSystem::Move(const Transform &transform, const AABBComponent &aabb, glm::vec3 movement)
    {
        glm::vec3 result{0.f};
        Transform current = transform;

        for (int i = 0; i < 3; ++i)
        {
            float allowed = MoveAxis(current, aabb, movement[i], i);
            current.position[i] += allowed;
            result[i] = allowed;
        }
        return result;
    }
}
