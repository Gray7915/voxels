#include "ChunkMeshSystem.hpp"
#include "World/Area.hpp"
#include "World/ChunkRenderer.hpp"

#include "Util/ThreadSafeQueue.hpp"

namespace lve
{
    static const glm::ivec3 DIRECTIONS[] = {
        {1, 0, 0},   // up
        {1, 0, 1},   // up right
        {0, 0, 1},   // right
        {-1, 0, 1},  // down right
        {-1, 0, 0},  // down
        {-1, 0, -1}, // down left
        {0, 0, -1},  // left
        {1, 0, 1}    // up left
    };

    ChunkMeshSystem::ChunkMeshSystem(Area &worldArea, LveDevice &device) : area{worldArea}, device{device}
    {
    }

    ChunkMeshSystem::~ChunkMeshSystem() = default;

    void ChunkMeshSystem::Update(LveDevice &device, int frameIndex)
    {

        for (auto &[coord, chunk] : area.AllChunks())
        {

            if (chunk->chunkState == ChunkState::Generated || chunk->chunkState == ChunkState::Dirty)
            {
                std::unordered_map<glm::ivec3, std::unique_ptr<lve::Chunk>, lve::IVec3Hash> neightbors = area.AllChunks();
                for (glm::ivec3 direction : DIRECTIONS)
                {
                    auto chunkNeighbor = neightbors.find(coord + direction);

                    if (chunkNeighbor != neightbors.end() && chunkNeighbor->second->voxelData.isGenerated())
                    {
                        auto &neighborChunk = chunkNeighbor->second;
                        VoxelData neightborChunkData = neighborChunk->voxelData;
                    }
                }
                tryQueueForMeshing(coord, *chunk, device);
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
                std::cout << "Dropped mesh/gen result for chunk " << result.chunkCoord.x << ", " << result.chunkCoord.y << ", " << result.chunkCoord.z << "\n";
                continue;
            }
            // std::cout << "verts: " << result.verticies.size()<< " indices: " << result.indices.size() << std::endl;
            chunk->applyMesh(std::move(result.model), frameIndex, device);
            chunk->chunkState = ChunkState::Uploaded;
        }
    }

    void ChunkMeshSystem::tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk, LveDevice &device)
    {
        MeshJob job;
        job.chunkCoord = coord;
        job.worldOffset = chunk.offset;
        job.voxelData = chunk.voxelData;
        job.device = &device;

        chunk.chunkState = ChunkState::QueuedForMeshing;
        meshPool.submit(std::move(job));
    }

    void ChunkMeshSystem::getNeighborChunkInfo(glm::ivec3 chunkDir, VoxelData chunkData, NeighborVoxelInfo &neighborChunkInfo)
    {   
        if (chunkDir == glm::ivec3{1, 0, 0}){ // up
            //x == 0, loop z and y -> up right corner has x = 0, z = 0 loop y
        }else if(chunkDir == glm::ivec3{1, 0, 1}){ // up right
            //x == 0, z == 0 loop y
        }
    }
}
