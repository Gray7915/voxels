#pragma once
#include <glm/glm.hpp>

enum class ColliderType
{
    AABB,
    Sphere,
    OBB,
};

struct AABBComponent
{
    ColliderType type = ColliderType::AABB;
    glm::vec3 halfExtents{0.5f};
    bool isTrigger = false;
    bool collisionEnabled = true;
    float radius;
};
