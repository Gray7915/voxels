#pragma once
#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/ChunkGenWorkerPool.cpp"
#include "World/Area.hpp"

namespace lve
{
    class ChunkGenerationSystem
    {
    public:
        void update(Area &area)
        {
            GenResult result;
            int budget = 4;
            while (budget-- > 0 && genPool.resultQueue.try_pop(result))
            {
                Chunk *chunk = area.getChunk(result.chunkCoord);
                if (!chunk)
                    continue; // chunk may have been unloaded while job was in flight
                chunk->setVoxelData(std::move(result.data));
                chunk->chunkState = ChunkState::Generated;
            }
        }
        void requestGeneration(glm::ivec3 coord)
        {
            genPool.jobQueue.push({coord});
        }

    private:
        ChunkGenWorkerPool genPool;
    };
}