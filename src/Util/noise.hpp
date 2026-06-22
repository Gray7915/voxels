#pragma once
#include "stb_perlin.h"
#include <glm/glm.hpp>
#include <cstdint>

namespace lve
{
    struct Noise
    {
        virtual float sample(glm::vec2 i) const = 0;
        virtual ~Noise() = default;
    };

    struct Octave : Noise
    {
        uint64_t seed;
        size_t n;
        float o;

        Octave(uint64_t seed, size_t n, float o)
            : seed(seed), n(n), o(o) {}

        float sample(glm::vec2 i) const override;
    };

    struct Combined : Noise
    {
        Noise *n;
        Noise *m;

        Combined(Noise &n, Noise &m)
            : n(&n), m(&m) {}

        float sample(glm::vec2 i) const override;
    };
}
