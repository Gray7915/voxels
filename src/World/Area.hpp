#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>
#include "Chunk.hpp"
#include "Util/IVec3Hash.h"

namespace lve
{
    class LveDevice;

    class Area
    {
    public:
        static const int MinMaxOffset = 2;

        Area();
        Area(LveDevice &lveDevice, glm::vec3 offset);
        ~Area();
        Area(const Area &) = delete;
        Area &operator=(const Area &) = delete;
        Area(Area &&) = default;
        Area &operator=(Area &&) = default;

        glm::vec3 areaCenter;

        void tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex);
        bool isBlockSolid(glm::vec3 worldBlockPos);

        Chunk *getChunk(glm::ivec3 coord);
        const Chunk *getChunk(glm::ivec3 coord) const;
        Chunk &getOrCreateChunk(glm::ivec3 coord, LveDevice &lveDevice);
        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> &AllChunks()
        {
            return chunks;
        }

    private:
        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> chunks;
    };
}