#pragma once
#include <glm/glm.hpp>

struct GravityComponent {
    glm::vec3 force = {0, -15, 0};
};
