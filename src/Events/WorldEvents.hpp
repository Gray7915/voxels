#pragma once

#include <glm/glm.hpp>
#include "ECS/Type.hpp"
#include "Util/Types.hpp"

namespace lve
{
    struct BlockBrokenRequest
    {
        ivec3 chunkPos;
        ivec3 blockPos;
        Entity brokenBy;
        BlockId blockID;
    };

    struct BlockPlaceRequest
    {
        ivec3 blockPos;
        ivec3 chunkPos;
        int blockType;
        Entity placedBy;
    };

    struct BlockHarvested
    {
        Entity brokenBy;
        int itemType;
    };
}
