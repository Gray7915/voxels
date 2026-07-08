#include "noise.hpp"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
namespace lve
{
    float Octave::sample(glm::vec2 i) const
    {
        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (size_t j = 0; j < n; j++)
        {
            float sx = (i.x * frequency) + (seed + j) * 12.9898f;
            float sy = (i.y * frequency) + (seed + j) * 78.233f;

            value += stb_perlin_noise3(sx, sy, 0.0f, 0, 0, 0) * amplitude;

            amplitude *= 0.2f; // less influence each octave
            frequency *= 0.6f; // more detail each octave
        }

        return value;
    }

    float Combined::sample(glm::vec2 i) const
    {
        return n->sample(glm::vec2(i.x + m->sample(i), i.y));
    }
}
