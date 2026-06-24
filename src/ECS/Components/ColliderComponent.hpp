#pragma once
#include <glm/glm.hpp>

enum class ColliderType
{
    AABB,
    Sphere,
    OBB,
};

struct ColliderComponent{
    ColliderType type = ColliderType::AABB;
    glm::vec3 halfExtents{0.5f};
    float radius = 0.5;
    bool isTrigger = false;
};
