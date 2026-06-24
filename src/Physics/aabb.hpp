#pragma once

#include <glm/glm.hpp>
#include "lve_game_object.hpp"

namespace lve
{
    class CollisionDetection
    {
        struct ContactPoint
        {
            glm::vec3 localA;
            glm::vec3 localB;
            glm::vec3 normal;
            float penetration;
        };

        struct CollisionInfo
        {
            LveGameObject *a;
            LveGameObject *b;

            ContactPoint point;

            void AddContactPoint(const glm::vec3 &localA, const glm::vec3 &localB, const glm::vec3 &normal, float p)
            {
                point.localA = localA;
                point.localB = localB;
                point.normal = normal;
                point.penetration = p;
            }
        };

        static bool ObjectIntersection(LveGameObject *a, LveGameObject *b, CollisionInfo &collisionInfo)
        {
        }
    };
}
