#pragma once
#include "Util/Types.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <FastNoiseLite.hpp>
#include <iostream>

namespace lve
{
    struct Noise {
        virtual float sample(vec2 i, s64 seed) const = 0;
        virtual float sample3d(vec3 i, s64 seed) const = 0;
        virtual ~Noise() = default;
    };

    struct Octave : Noise {
        size_t n;
        float o;
        FastNoiseLite fnl;

        Octave() : Octave(0, 4, 0.5f) {}
        Octave(u64 worldSeed, size_t n, float o) : n(n), o(o) {
            fnl.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            fnl.SetSeed((int)worldSeed);
            fnl.SetFractalType(FastNoiseLite::FractalType_FBm);
            fnl.SetFractalOctaves((int)n);
            fnl.SetFractalLacunarity(2.0f);
            fnl.SetFractalGain(o);
            fnl.SetFrequency(0.5f);
        }

        float sample(vec2 i, s64 worldSeed) const override;
        float sample3d(vec3 i, s64 worldSeed) const override;
    };

    struct Combined : Noise {
        Noise *n;
        Noise *m;

        Combined(Noise &n, Noise &m) : n(&n), m(&m) {}

        float sample(glm::vec2 i, s64 seed) const override;
    };
} // namespace lve