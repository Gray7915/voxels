#include "ChunkMeshSystem.hpp"
#include "World/Area.hpp"
#include "World/ChunkRenderer.hpp"

#include "Util/ThreadSafeQueue.hpp"

namespace lve
{

    ChunkMeshSystem::ChunkMeshSystem(Area &worldArea) : area{worldArea}
    {
    }

    ChunkMeshSystem::~ChunkMeshSystem() = default;

    void ChunkMeshSystem::Update(LveDevice &device)
    {
        // std::cout << "enterd mesh update" << '\n';

        for (auto &[coord, chunk] : area.AllChunks())
        {
            // std::cout << "entered chunks in area loop" << '\n';

            if (chunk->chunkState == ChunkState::Generated || chunk->chunkState == ChunkState::Dirty)
            {
                // std::cout << "try queue for mesh " << '\n';
                tryQueueForMeshing(coord, *chunk);
            }
        }
        

        MeshResult result;
        int budget = 4;
        // meshPool.printJobQueueSize();
        // meshPool.printResultQueue();
        while (budget-- > 0 && meshPool.tryGetResult(result))
        {
            // std::cout << "chunk coord in mesh while loop " << result.chunkCoord.x << ", " << result.chunkCoord.y << ", " << result.chunkCoord.z << '\n';
            Chunk *chunk = area.getChunk(result.chunkCoord);
            // std::cout << "chunk address " << chunk << '\n';
            if (!chunk)
            {
                std::cout << "Dropped mesh/gen result for chunk "
                          << result.chunkCoord.x << ", "
                          << result.chunkCoord.y << ", "
                          << result.chunkCoord.z << "\n";
                continue;
            }
            // std::cout << "verts: " << result.verticies.size()<< " indices: " << result.indices.size() << std::endl;
            chunk->uploadMesh(device, result.verticies, result.indices);
            chunk->chunkState = ChunkState::Uploaded;
        }
    }

    void ChunkMeshSystem::tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk)
    {
        MeshJob job;
        job.chunkCoord = coord;
        job.worldOffset = chunk.offset;
        job.voxelData = chunk.voxelData;

        chunk.chunkState = ChunkState::QueuedForMeshing;
        meshPool.submit(std::move(job));
    }
}
