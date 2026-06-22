// IVec3Hash.h
#pragma once
#include <glm/glm.hpp>
#include <bits/functional_hash.h>

namespace lve {
    struct IVec3Hash {
        size_t operator()(const glm::ivec3& v) const {
            size_t seed = 0;
            for (int i = 0; i < 3; i++)
                seed ^= std::hash<int>()(v[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}
