#pragma once
#include <glm/glm.hpp>

struct AABBComponent
{
    glm::vec3 halfExtents{0.5f};
    bool isTrigger = false;
    bool collisionEnabled = true;
};
