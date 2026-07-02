#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>
#include "Chunk.hpp"

namespace lve
{
    class Area
    {
    public:
        static const int MinMaxOffset = 4;
        Area();
        Area(LveDevice &lveDevice, glm::vec3 offset);
        ~Area();

        Area(const Area &) = delete;
        Area &operator=(const Area &) = delete;

        Area(Area &&) = default;
        Area &operator=(Area &&) = default;

        glm::vec3 areaCenter;
        static std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> chunks;
        void tick(LveDevice &lveDevice, glm::vec3 center);
        static bool isBlockSolid(glm::vec3 worldBlockPos);
        static void reMeshChunk(glm::ivec3 chunkPosition);
    };
}
