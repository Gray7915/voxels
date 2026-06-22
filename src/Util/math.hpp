#include <glm/glm.hpp>
#include <glm/common.hpp>
#include "../World/Chunk.hpp"

namespace lve
{

    static glm::ivec3 WorldToChunkId(glm::vec3 worldCoordinate)
    {
        glm::vec3 floored = glm::floor(worldCoordinate);
        glm::vec3 divided = floored / glm::vec3(Chunk::CHUNK_SIZE);
        return glm::ivec3(glm::floor(divided));
    }

    static glm::ivec3 WorldToChunkArray(glm::vec3 worldCoordinate)
    {
        glm::ivec3 p = glm::floor(worldCoordinate);
        glm::ivec3 c = Chunk::CHUNK_SIZE;

        glm::ivec3 r = p % c;
        return (r + c) % c;
    }
}
