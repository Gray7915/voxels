#pragma once
#include <glm/glm.hpp>
#include "Util/Types.hpp"
namespace lve
{
    struct BoxVolume
    {
        vec3 boxPosition;
        vec3 boxSize;
        vec3 offset;
    };
}
