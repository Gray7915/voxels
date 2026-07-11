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
        {1, 0, -1}   // up left
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
                NeighborVoxelInfo neighborVoxelInfo;
                neighborVoxelInfo.allocate();
                auto &neighbors = area.AllChunks();
                for (glm::ivec3 direction : DIRECTIONS)
                {
                    auto chunkNeighbor = neighbors.find(coord + direction);
                    VoxelData neighborChunkData;
                    neighborChunkData.allocate();
                    if (chunkNeighbor != neighbors.end() && chunkNeighbor->second->voxelData.isGenerated())
                    {
                        auto &neighborChunk = chunkNeighbor->second;
                        neighborChunkData = neighborChunk->voxelData;
                    }
                    getNeighborChunkInfo(direction, neighborChunkData, neighborVoxelInfo);
                }
                tryQueueForMeshing(coord, *chunk, device, neighborVoxelInfo);
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
                // std::cout << "Dropped mesh/gen result for chunk " << result.chunkCoord.x << ", " << result.chunkCoord.y << ", " << result.chunkCoord.z << "\n";
                continue;
            }
            // std::cout << "verts: " << result.verticies.size()<< " indices: " << result.indices.size() << std::endl;
            chunk->applyMesh(std::move(result.model), frameIndex, device);
            chunk->chunkState = ChunkState::Uploaded;
            chunk->indicies = result.indices.size();
            chunk->verticies = result.verticies.size();
        }
    }

    void ChunkMeshSystem::tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk, LveDevice &device, NeighborVoxelInfo neighborVoxelInfo)
    {
        MeshJob job;
        job.chunkCoord = coord;
        job.worldOffset = chunk.offset;
        job.voxelData = chunk.voxelData;
        job.neighborVoxelData = neighborVoxelInfo;
        job.device = &device;

        chunk.chunkState = ChunkState::QueuedForMeshing;
        meshPool.submit(std::move(job));
    }

    void ChunkMeshSystem::getNeighborChunkInfo(glm::ivec3 chunkDir, VoxelData chunkData, NeighborVoxelInfo &neighborChunkInfo)
    {
        if (chunkDir == glm::ivec3{1, 0, 0})
        { // up
            // x == 0, loop z and y
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    neighborChunkInfo.set(z, y, 0, chunkData.get(0, y, z));
                }
            }
        }
        else if (chunkDir == glm::ivec3{1, 0, 1})
        { // up right
          // x == 0, z == 0 loop y
            for (int y = 0; y < VoxelData::HEIGHT; y++)
            {
                neighborChunkInfo.set(16, y, 0, chunkData.get(0, y, 0));
            }
        }
        else if (chunkDir == glm::ivec3{0, 0, 1})
        { // right
            // z == 0, loop x and y
            for (int x = 0; x < VoxelData::DEPTH; x++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    neighborChunkInfo.set(x, y, 1, chunkData.get(x, y, 0));
                }
            }
        }
        else if (chunkDir == glm::ivec3{-1, 0, 1})
        { // down right
          // x == 0, z == 0 loop y
            for (int y = 0; y < VoxelData::HEIGHT; y++)
            {
                neighborChunkInfo.set(16, y, 1, chunkData.get(15, y, 0));
            }
        }
        else if (chunkDir == glm::ivec3{-1, 0, 0})
        { // down
            // x == 15, loop z and y
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    neighborChunkInfo.set(z, y, 2, chunkData.get(15, y, z));
                }
            }
        }
        else if (chunkDir == glm::ivec3{-1, 0, -1})
        { // down left
          // x == 15, z == 15 loop y
            for (int y = 0; y < VoxelData::HEIGHT; y++)
            {
                neighborChunkInfo.set(16, y, 2, chunkData.get(15, y, 15));
            }
        }
        else if (chunkDir == glm::ivec3{0, 0, -1})
        { // left
            // x == 15, loop z and y
            for (int x = 0; x < VoxelData::DEPTH; x++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    neighborChunkInfo.set(x, y, 3, chunkData.get(x, y, 15));
                }
            }
        }
        else if (chunkDir == glm::ivec3{1, 0, -1})
        { // up left
          // x == 15, z == 15 loop y
            for (int y = 0; y < VoxelData::HEIGHT; y++)
            {
                neighborChunkInfo.set(16, y, 3, chunkData.get(0, y, 15));
            }
        }
    }

    /*

    // How we get the border

    [leftUp][up][up][up][up][upR]           // Up to Up right is z = 0, x = 0 - 16
    [left]                    [right]       // right to right down is z = 1, x = 0 - 16
    [left]      center        [right]       // down to down left is z = 2, x = 0 - 16
    [left]      chunk         [right]       // left to up left is z = 3, x = 0 - 16
    [left]                    [right]       // Y is top to bottom of that chunk slice
    [dwnl][dwn][dwn][dwn][dwn][rightD]


    // How we store the border
    z = 0                       z = 1                           z = 2                           z = 3 <- border z coordinate

    These are world pos not chunk pos
    [up x = 0, z = 4]           [right x = 4, z = 3]            [down x = 3, z = -1]            [left x = -1, z = 0]        x = 0
    [up x = 1, z = 4]           [right x = 4, z = 2]            [down x = 2, z = -1]            [left x = -1, z = 1]        x = 1
    [up x = 2, z = 4]           [right x = 4, z = 1]            [down x = 1, z = -1]            [left x = -1, z = 2]        x = 2
    [up x = 3, z = 4]           [right x = 4, z = 0]            [down x = 0, z = -1 ]           [left x = -1, z = 3]        x = 3
    [upRight x = 4, z = 4]      [rightDown x = 4, z = -1]       [downLeft x = -1, z = -1]       [leftUp x = -1, z = 4]      x = 4
                                                                                                                            ^ x border coordinate                
    */
}
