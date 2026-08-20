#include "noise.hpp"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include <FastNoiseLite.hpp>
#include <iostream>
namespace lve
{
    float Octave::sample(vec2 i, s64 worldSeed) const { return fnl.GetNoise(i.x, i.y); }

    float Combined::sample(vec2 i, s64 seed) const { return n->sample(glm::vec2(i.x + m->sample(i, seed), i.y), seed); }
} // namespace lve
