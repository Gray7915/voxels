
#include <glm/glm.hpp>
#include "glm/gtx/norm.hpp"
#include <iostream>
#include <functional>
#include "../World/Area.hpp"
#include "../Util/math.hpp"

namespace lve
{
    class Ray
    {
    public:
        glm::vec3 origin, direction;

        Ray() = default;

        Ray(glm::vec3 origin, glm::vec3 direction)
        {

            this->origin = origin;
            this->direction = direction;

            // std::cout << "Ray origin " << origin.x << " " << origin.y << " " << origin.z << '\n';
            // std::cout << "Ray direction (normalized)" << direction.x << " " << direction.y << " " << direction.z << '\n';

            glm::ivec3 p, step, d;
            glm::vec3 t_max, t_delta;
            float radius;

            p = glm::floor(origin);
            step = glm::sign(direction);
            t_max = intbound(origin, direction);
            t_delta = glm::vec3(step) / direction;
            radius = 4 / glm::l2Norm(direction);
        }

        glm::vec3 detectBlockHit(float max_distance)
        {
            const glm::vec3 step = glm::sign(direction);

            glm::vec3 delta;
            delta.x = (direction.x != 0) ? std::abs(1.0f / direction.x) : 1e30f;
            delta.y = (direction.y != 0) ? std::abs(1.0f / direction.y) : 1e30f;
            delta.z = (direction.z != 0) ? std::abs(1.0f / direction.z) : 1e30f;

            glm::ivec3 pos = glm::floor(origin);
            glm::vec3 tmax = intbound(origin, direction);

            int axis = 0;

            for (int steps = 0; steps < 64; ++steps)
            {

                uint32_t voxel = 0;
                // std::cout<< "Checking voxel "<< pos.x << " "<< pos.y << " "<< pos.z<< " value=" << voxel<< '\n';

                glm::ivec3 chunkPos(
                    std::floor(pos.x / 16.0f),
                    std::floor(pos.y / 32.0f),
                    std::floor(pos.z / 16.0f));

                auto it = Area::chunks.find(chunkPos);
                if (it != Area::chunks.end() && it->second)
                {
                    glm::ivec3 arrayPos = WorldToChunkArray(pos);
                    voxel = it->second->blocks[arrayPos.x][arrayPos.y][arrayPos.z];
                }

                if (voxel)
                {
                    if (steps == 0)
                        return origin;
                    // std::cout << "Chunk " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << '\n';

                    float tHit = tmax[axis] - delta[axis];
                    return pos;
                }

                if (tmax.x < tmax.y)
                {
                    if (tmax.x < tmax.z)
                    {
                        pos.x += step.x;
                        axis = 0;
                        tmax.x += delta.x;
                    }
                    else
                    {
                        pos.z += step.z;
                        axis = 2;
                        tmax.z += delta.z;
                    }
                }
                else
                {
                    if (tmax.y < tmax.z)
                    {
                        pos.y += step.y;
                        axis = 1;
                        tmax.y += delta.y;
                    }
                    else
                    {
                        pos.z += step.z;
                        axis = 2;
                        tmax.z += delta.z;
                    }
                }

                if (std::min({tmax.x, tmax.y, tmax.z}) > max_distance)
                    break;
            }

            return glm::vec3(-1.0f);
        }

        glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
        {
            glm::vec3 res;
            for (size_t i = 0; i < 3; i++)
            {
                res[i] =
                    (ds[i] > 0 ? (glm::ceil(s[i]) - s[i]) : (s[i] - glm::floor(s[i]))) / glm::abs(ds[i]);
            }
            return res;
        }
    };
}
