#include "ChunkMeshSystem.hpp"
#include "World/Area.hpp"
#include "World/ChunkRenderer.hpp"

#include "Util/ThreadSafeQueue.hpp"

namespace lve
{

    ChunkMeshSystem::ChunkMeshSystem(Area &worldArea) : area{worldArea}
    {
    }

    void ChunkMeshSystem::Update(LveDevice &device, uint32_t currentFrameIndex)
    {
        for (auto &[coord, chunk] : area.AllChunks())
        {
            if (chunk->chunkState == ChunkState::Generated)
            {
                tryQueueForMeshing(coord, *chunk);
            }
        }

        MeshResult result;
        int budget = 4;
        while (budget-- > 0 && meshPool.tryGetResult(result))
        {
            Chunk *chunk = area.getChunk(result.chunkCoord);
            if (!chunk)
                continue;

            chunk->uploadMesh(device, result.verticies, result.indices);
            chunk->chunkState = ChunkState::Uploaded;
        }
    }

    void ChunkMeshSystem::tryQueueForMeshing(glm::ivec3 coord, Chunk chunk)
    {
        meshPool.jobQueue.push({chunk.offset});
        chunk.chunkState = ChunkState::QueuedForMeshing;
    }
}
