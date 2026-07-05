#include "World/Systems/ChunkGenerationSystem.hpp"

namespace lve
{
    ChunkGenerationSystem::ChunkGenerationSystem(Area &area) : area{area}
    {
    }

    ChunkGenerationSystem::~ChunkGenerationSystem() = default;

    void ChunkGenerationSystem::update(Area &area)
    {
        std::cout << "try gen chunk" << '\n';

        GenResult result;
        std::cout << "create result" << '\n';

        int budget = 4;
        while (budget-- > 0 && genPool.resultQueue.try_pop(result))
        {
            std::cout << "in try gen chunk loop" << '\n';
            Chunk *chunk = area.getChunk(result.chunkCoord);
            if (!chunk)
                continue; // chunk may have been unloaded while job was in flight
            std::cout << "generated chunk" << '\n';
            chunk->setVoxelData(std::move(result.data));
            chunk->chunkState = ChunkState::Generated;
        }
    }
    void ChunkGenerationSystem::requestGeneration(glm::ivec3 coord)
    {
        genPool.jobQueue.push({coord});
    }
}