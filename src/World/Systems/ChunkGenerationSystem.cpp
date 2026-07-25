#include "World/Systems/ChunkGenerationSystem.hpp"
#include "World/Area.hpp"

namespace lve
{
    ChunkGenerationSystem::ChunkGenerationSystem(Area &area) : area{area}
    {
    }

    ChunkGenerationSystem::~ChunkGenerationSystem() = default;

    void ChunkGenerationSystem::update()
    {
        for (auto &[coord, chunk] : area.AllChunks())
        {
            if (chunk->chunkState == ChunkState::Unloaded)
            {
                chunk->chunkState = ChunkState::QueuedForGeneration;
                requestGeneration(chunk->offset);
            }
        }

        GenResult result;
        int budget = 4;
        while (budget-- > 0 && genPool.tryGetResult(result))
        {
            Chunk *chunk = area.getChunk(result.chunkCoord);
            if (!chunk)
                continue;

            chunk->setVoxelData(std::move(result.data));
            chunk->chunkState = ChunkState::Generated;
            area.markNeighborChunksDirty(result.chunkCoord);
        }
    }

    void ChunkGenerationSystem::requestGeneration(glm::ivec3 coord)
    {
        genPool.submit({coord});
    }
}
