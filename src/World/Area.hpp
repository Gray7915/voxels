#pragma once
#include "Chunk.hpp"
#include "Util/Direction.hpp"
#include "Util/IVec3Hash.h"
#include "Util/Types.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include "ECS/Coordinator.hpp"

#include "Rendering/Core/lve_device.hpp"
#include "World/Systems/ChunkGenerationSystem.hpp"
#include "World/Systems/ChunkMeshSystem.hpp"
#include "World/Systems/ChunkMutationSystem.hpp"
namespace lve
{
    class ChunkMeshSystem;
    class Area {
      public:
        static const int MinMaxOffset = 4;

        Area();
        Area(LveDevice &lveDevice, s64 seed, Coordinator &coordinator);
        ~Area();
        Area(const Area &) = delete;
        Area &operator=(const Area &) = delete;
        Area(Area &&) = default;
        Area &operator=(Area &&) = default;

        void setWorldSeed(s64 worldSeed) { this->worldSeed = worldSeed; }
        s64 getWorldSeed() { return worldSeed; }
        vec3 areaCenter;

        void tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex);
        void markNeighborChunksDirty(ivec3 centerChunkPos);
        void markChunkDity(ivec3 chunkPos);
        void setBlockAtPos(ivec3 blockWorldPos, BlockId id);

        bool isBlockSolid(glm::vec3 worldBlockPos);
        bool isBlockSolid(glm::vec3 worldBlockPos, glm::vec3 rayPos, glm::vec3 rayDirection);
        uint16_t getBlockID(glm::vec3 worldBlockPos);

        Chunk *getChunk(glm::ivec3 coord);
        const Chunk *getChunk(glm::ivec3 coord) const;
        Chunk &getOrCreateChunk(glm::ivec3 coord, LveDevice &lveDevice, ChunkGenerationSystem &chunkGenSystem);

        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> &AllChunks() { return chunks; }
        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> chunks;

        void updateArea();

      private:
        s64 worldSeed;

        LveDevice &device;

        ChunkGenerationSystem chunkGenSystem;
        ChunkMeshSystem chunkMeshSystem;
        ChunkMutationSystem chunkMutationSystem;

        Coordinator &coordinator;
    };
} // namespace lve

#include "Physics/aabb.hpp"
