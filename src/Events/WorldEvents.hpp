#pragma once

#include <glm/glm.hpp>
#include "ECS/Type.hpp"

namespace lve
{
    struct BlockBrokenEvent
    {
        glm::ivec3 chunkPos;
        glm::ivec3 blockPos;
        Entity brokenBy;
    };

    struct BlockPlaceRequest
    {
        glm::ivec3 blockPos;
        glm::ivec3 chunkPos;
        int blockType;
        Entity placedBy;
    };

    struct BlockHarvested
    {
        Entity brokenBy;
        int itemType;
    };
}
