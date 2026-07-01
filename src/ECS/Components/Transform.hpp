#pragma once

#include <glm/glm.hpp>
namespace lve
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
}
#include "Transform.inl"
