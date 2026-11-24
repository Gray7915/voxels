#include <glm/gtc/matrix_access.hpp>

#include "Util/Frustrum.hpp"

void Frustum::update(const glm::mat4 &m)
{
    glm::vec4 row0 = glm::row(m, 0);
    glm::vec4 row1 = glm::row(m, 1);
    glm::vec4 row2 = glm::row(m, 2);
    glm::vec4 row3 = glm::row(m, 3);

    auto makePlane = [](glm::vec4 p)
    {
        Plane3D plane;
        plane.normal = glm::vec3(p);
        plane.d = p.w;
        plane.normalize();
        return plane;
    };

    planes[Left] = makePlane(row3 + row0);
    planes[Right] = makePlane(row3 - row0);
    planes[Bottom] = makePlane(row3 - row1);
    planes[Top] = makePlane(row3 + row1);
    planes[Near] = makePlane(row2);
    planes[Far] = makePlane(row3 - row2);
}

bool Frustum::intersectsAABB(glm::vec3 min, glm::vec3 max) const
{
    for (const auto &plane : planes)
    {
        glm::vec3 positive = min;

        if (plane.normal.x >= 0)
            positive.x = max.x;
        if (plane.normal.y >= 0)
            positive.y = max.y;
        if (plane.normal.z >= 0)
            positive.z = max.z;

        if (plane.distance(positive) < 0)
            return false;
    }

    return true;
}
