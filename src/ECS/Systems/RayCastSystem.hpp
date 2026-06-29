#pragma once

#include "../Components/Input.hpp"
#include "../System.hpp"
#include "../Coordinator.hpp"

namespace lve
{
    class RayCastSystem : public System
    {
        void Init();

        void Update(float dt);

        

    private:
        glm::ivec3 p;
        glm::ivec3 step;
        glm::ivec3 d;
        glm::vec3 t_max;
        glm::vec3 t_delta;
        float radius;

        glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
        {
            return glm::mix(
                (s - glm::floor(s)) / glm::abs(ds),
                (glm::ceil(s) - s) / glm::abs(ds),
                glm::greaterThan(ds, glm::vec3(0.0f)));
        }

    };
}
