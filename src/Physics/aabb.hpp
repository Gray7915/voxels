#pragma once

#include <glm/glm.hpp>
#include "ECS/Components/AABBComponent.hpp"
#include "ECS/Components/Transform.hpp"
#include "World/Area.hpp"
#include "Util/lve_util.hpp"

namespace lve
{
    class CollisionDetection
    {
    public:
        struct ContactPoint
        {
            glm::vec3 localA;
            glm::vec3 localB;
            glm::vec3 normal;
            float penetration;
        };

        struct CollisionInfo
        {
            ContactPoint point;

            void AddContactPoint(const glm::vec3 &localA, const glm::vec3 &localB, const glm::vec3 &normal, float p)
            {
                point.localA = localA;
                point.localB = localB;
                point.normal = normal;
                point.penetration = p;
            }
        };

        // TODO: placing a block at head height will place and trap player
        static bool CheckBlockPlacement(const Transform &transform, const AABBComponent &aabbComponent, glm::ivec3 position)
        {
            glm::vec3 minPos = transform.position - aabbComponent.halfExtents;
            glm::vec3 maxPos = transform.position + aabbComponent.halfExtents;

            glm::ivec3 minBlock = glm::floor(minPos);
            glm::ivec3 maxBlock = glm::floor(maxPos);

            for (int x = minBlock.x; x <= maxBlock.x; ++x)
                for (int y = minBlock.y; y <= maxBlock.y; ++y)
                    for (int z = minBlock.z; z <= maxBlock.z; ++z)
                    {
                        if (glm::ivec3(glm::floor(glm::vec3(x, y, z))) == position)
                        {
                            return true;
                        }
                    }
            return false;
        }

        static bool CheckTerrainOverlap(const Transform &transform, const AABBComponent &aabbComponent, Area &area)
        {
            glm::vec3 minPos = transform.position - aabbComponent.halfExtents;
            glm::vec3 maxPos = transform.position + aabbComponent.halfExtents;

            glm::ivec3 minBlock = glm::floor(minPos);
            glm::ivec3 maxBlock = glm::floor(maxPos);
            for (int x = minBlock.x; x <= maxBlock.x; ++x)
                for (int y = minBlock.y; y <= maxBlock.y; ++y)
                    for (int z = minBlock.z; z <= maxBlock.z; ++z)
                    {
                        if (area.isBlockSolid(glm::ivec3(x, y, z)))
                        {
                            return true;
                        }
                    }
            return false;
        }

        static float MoveAxis(const Transform &transform, const AABBComponent &aabb, float movement, int axis, Area &area)
        {
            constexpr float EPSILON = 0.05f;

            glm::vec3 testPos = transform.position;
            testPos[axis] += movement;

            Transform testTransform = transform;
            testTransform.position = testPos;

            if (!CheckTerrainOverlap(testTransform, aabb, area))
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

                if (CheckTerrainOverlap(testTransform, aabb, area))
                    hi = mid;
                else
                    lo = mid;
            }

            float allowed = lo - sign * EPSILON;
            return (glm::abs(allowed) <= EPSILON) ? 0.f : allowed;
        }

        static glm::vec3 Move(const Transform &transform, const AABBComponent &aabb, glm::vec3 movement, Area &area)
        {
            glm::vec3 result{0.f};
            Transform current = transform;

            for (int i = 0; i < 3; ++i)
            {
                float allowed = MoveAxis(current, aabb, movement[i], i, area);
                current.position[i] += allowed;
                result[i] = allowed;
            }
            return result;
        }

        static bool rayBoxIntersection(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 boxPos, glm::vec3 boxSize)
        {

            // boxPos.x = boxPos.x + 0.5f;
            // boxPos.z = boxPos.z + 0.5f;

            boxPos.x += 0.5f;
            boxPos.z += 0.5f;
            //std::cout << "boxPos " << boxPos.x << " " << boxPos.y << " " << boxPos.z << '\n';

            glm::vec3 boxMin = boxPos - (boxSize / glm::vec3(2, 1, 2));
            glm::vec3 boxMax = boxPos + (boxSize / glm::vec3(2, 1, 2));
            //std::cout << "min: " << boxMin.x << " " << boxMin.y << " " << boxMin.z << '\n';
            //std::cout << "max: " << boxMax.x << " " << boxMax.y << " " << boxMax.z << '\n';

            glm::vec3 tVals(-1, -1, -1);

            glm::vec3 t1 = (boxMin - rayPos) / rayDir;
            glm::vec3 t2 = (boxMax - rayPos) / rayDir;

            glm::vec3 tMin = glm::min(t1, t2);
            glm::vec3 tMax = glm::max(t1, t2);

            float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
            float tFar = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

            return tFar >= 0.0f && tNear <= tFar;
        }
    };
}
