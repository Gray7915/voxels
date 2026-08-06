#pragma once

#include "Util/Types.hpp"

struct Plane3D
{
    vec3 normal;
    float d;

    void normalize()
    {
        float len = glm::length(normal);
        normal /= len;
        d /= len;
    }

    float distance(vec3 p) const
    {
        return glm::dot(normal, p) + d;
    }
};

class Frustum
{
public:
    Plane3D planes[6];

    enum
    {
        Left,
        Right,
        Bottom,
        Top,
        Near,
        Far
    };

    void update(const mat4 &vp);
    bool intersectsAABB(glm::vec3 min, glm::vec3 max) const;
};
