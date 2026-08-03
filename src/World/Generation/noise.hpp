#pragma once
#include "Util/Types.hpp"
#include "stb_perlin.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace lve
{
    struct Noise {
        virtual float sample(glm::vec2 i, u64 seed) const = 0;
        virtual ~Noise() = default;
    };

    struct Octave : Noise {
        uint64_t seed;
        size_t n;
        float o;
        Octave() : Octave(0, 4, 0.5f) {}
        Octave(uint64_t worldSeed, size_t n, float o) : seed(worldSeed), n(n), o(o) {}

        float sample(glm::vec2 i, u64 worldSeed) const override;
    };

    struct Combined : Noise {
        Noise *n;
        Noise *m;

        Combined(Noise &n, Noise &m) : n(&n), m(&m) {}

        float sample(glm::vec2 i, u64 seed) const override;
    };
} // namespace lve
