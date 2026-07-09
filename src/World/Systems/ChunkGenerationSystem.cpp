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
        // 1. Sweep for newly-created chunks that need generation
        for (auto &[coord, chunk] : area.AllChunks())
        {
            if (chunk->chunkState == ChunkState::Unloaded)
            {
                chunk->chunkState = ChunkState::QueuedForGeneration;
                requestGeneration(chunk->offset);
            }
        }

        // 2. Drain finished generation results
        GenResult result;
        int budget = 4;
        while (budget-- > 0 && genPool.tryGetResult(result))
        {
            Chunk *chunk = area.getChunk(result.chunkCoord);
            if (!chunk)
                continue; // chunk was unloaded while its job was in flight — discard

            chunk->setVoxelData(std::move(result.data));
            chunk->chunkState = ChunkState::Generated;
            //std::cout << "generated chunk " << result.chunkCoord.x << ", "<< result.chunkCoord.y << ", " << result.chunkCoord.z << '\n';
        }
    }

    void ChunkGenerationSystem::requestGeneration(glm::ivec3 coord)
    {
        // std::cout << "Queued " << coord.x << ", " << coord.y << ", " << coord.z << '\n';
        genPool.submit({coord});
    }
}
