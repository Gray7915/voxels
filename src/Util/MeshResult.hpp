#pragma once
#include <glm/glm.hpp>
#include "Util/lve_util.hpp"

namespace lve
{
    struct MeshResult
    {
        glm::ivec3 ChunkCoord;
        std::vector<Vertex> verticies;
        std::vector<uint32_t> indices;
    };

}