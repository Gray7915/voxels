#pragma once
#include <glm/glm.hpp>

struct RayCastComponent
{
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hitPosition;
    glm::vec3 hitDirection;
};
