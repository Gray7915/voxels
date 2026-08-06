#include "noise.hpp"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include <iostream>
namespace lve
{
    float Octave::sample(vec2 i, s64 worldSeed) const {
        float value = 0.0f;
        float amplitude = 0.30f;
        float frequency = 1.0f;

        for (size_t j = 0; j < n; j++) {
            // Hash seed + octave index into a small float offset
            uint64_t h = (uint64_t)(worldSeed + j);
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53ULL;
            h ^= h >> 33;

            float ox = (float)(h & 0xFFFF) / 65535.0f * 256.0f; // offset in [0, 256)
            float oy = (float)((h >> 16) & 0xFFFF) / 65535.0f * 256.0f;

            float sx = (i.x * frequency) + ox;
            float sy = (i.y * frequency) + oy;

            value += stb_perlin_noise3(sx, sy, 0.0f, 0, 0, 0) * amplitude;

            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return value;
    }

    float Combined::sample(vec2 i, s64 seed) const { return n->sample(glm::vec2(i.x + m->sample(i, seed), i.y), seed); }
} // namespace lve
