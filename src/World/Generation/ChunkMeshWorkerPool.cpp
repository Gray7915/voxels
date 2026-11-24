#include <iostream>
#include <optional>

#include "World/Generation/ChunkMeshWorkerPool.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/ConnectedBlock.hpp"
#include "World/Blocks/StagedBlock.hpp"
#include "World/Blocks/CreateVoxel.hpp"
#include "World/Blocks/VoxelEmitter.hpp"
#include "World/Blocks/WorldVoxelContext.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "App/TextureAtlas.hpp"

#include "Util/Direction.hpp"
#include "Util/Types.hpp"
#include "Util/Constants.hpp"

namespace lve
{
    static const size_t UNIQUE_INDICES[] = {0, 1, 2, 5};
    static const size_t FACE_INDICES[] = {0, 1, 2, 0, 2, 3};

    static const size_t CUBE_INDICES[] = {
        4, 7, 6, 4, 6, 5, // south (+z)
        3, 0, 1, 3, 1, 2, // north (-z)
        7, 3, 2, 7, 2, 6, // east  (+x)
        0, 4, 5, 0, 5, 1, // west  (-x)
        2, 1, 5, 2, 5, 6, // up    (+y)
        0, 3, 7, 0, 7, 4  // down  (-y)
    };

    static const vec3 CUBE_VERTICES[] = {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}};

    static const vec3 CUBE_NORMALS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

    static const vec2 CUBE_UVS[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    ChunkMeshWorkerPool::ChunkMeshWorkerPool(LveDevice &device, size_t threadCount) : device(device) {
        for (size_t i = 0; i < threadCount; i++)
            workers.emplace_back([this] {
                workerLoop();
            });
    }

    ChunkMeshWorkerPool::~ChunkMeshWorkerPool() {
        running = false;
        jobQueue.shutdown();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }

    void ChunkMeshWorkerPool::workerLoop() {
        VkCommandPool threadPool = device.createTransientCommandPool();

        MeshJob job;
        while (jobQueue.wait_and_pop(job)) {
            resultQueue.push(generateMesh(job, threadPool));
        }

        device.destroyCommandPool(threadPool);
    }

    MeshResult ChunkMeshWorkerPool::generateMesh(MeshJob &job, VkCommandPool pool) {
        MeshResult result{};
        result.chunkCoord = job.chunkCoord;

        Blocks::WorldVoxelContext ctx(job);
        Blocks::MeshOutput out{result.verticies, result.indices};

        for (int x = 0; x < VoxelData::WIDTH; x++)
            for (int z = 0; z < VoxelData::DEPTH; z++)
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                    if (job.voxelData.get(x, y, z) != 0)
                        Blocks::emitVoxel(ctx, out, ivec3{x, y, z});

        if (result.verticies.size() < 3) {
            result.model = nullptr;
            return result;
        }
        result.model = LveModel::createChunkModel(*job.device, result.verticies, result.indices, pool);
        return result;
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
} // namespace lve
