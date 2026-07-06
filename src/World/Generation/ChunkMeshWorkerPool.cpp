#include "World/Generation/ChunkMeshWorkerPool.hpp"
#include <iostream>

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

    static const glm::vec3 CUBE_VERTICES[] = {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}};

    static const glm::vec3 CUBE_NORMALS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

    static const glm::vec2 CUBE_UVS[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    static const glm::ivec3 DIRECTIONS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

    ChunkMeshWorkerPool::ChunkMeshWorkerPool(size_t threadCount)
    {
        for (size_t i = 0; i < threadCount; i++)
            workers.emplace_back([this]
                                 { workerLoop(); });
    }

    ChunkMeshWorkerPool::~ChunkMeshWorkerPool()
    {
        running = false;
        jobQueue.shutdown();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }

    void ChunkMeshWorkerPool::workerLoop()
    {
        while (running)
        {
            MeshJob job;
            if (!jobQueue.wait_and_pop(job))
                break;
            resultQueue.push(generateMesh(job));
        }
    }

    MeshResult ChunkMeshWorkerPool::generateMesh(MeshJob &job)
    {
        MeshResult result{};
        result.chunkCoord = job.chunkCoord;
        std::cout << "chunk coord in gen mesh " << job.chunkCoord.x << ", " << job.chunkCoord.y << ", " << job.chunkCoord.z << '\n';
        std::cout << "world offset in gen mesh " << job.worldOffset.x << ", " << job.worldOffset.y << ", " << job.worldOffset.z << '\n';

        for (int x = 0; x < VoxelData::WIDTH; x++)
        {
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    if (job.voxelData.get(x, y, z) == 0)
                        continue; // skip air — was missing entirely before

                    emitBlock(job, result, glm::ivec3(x, y, z));
                }
            }
        }
        return result;
    }

    void ChunkMeshWorkerPool::emitBlock(MeshJob &job, MeshResult &result, glm::ivec3 pos)
    {
        int blockType = job.voxelData.get(pos.x, pos.y, pos.z);

        for (int face = 0; face < 6; face++)
        {
            glm::ivec3 n = pos + getDirection(face);
            bool visible = n.x < 0 || n.y < 0 || n.z < 0 ||
                           n.x >= VoxelData::WIDTH || n.y >= VoxelData::HEIGHT || n.z >= VoxelData::DEPTH ||
                           job.voxelData.get(n.x, n.y, n.z) == 0 ||
                           job.voxelData.get(n.x, n.y, n.z) == 4;

            if (!visible)
                continue;

            const uint32_t baseIndex = static_cast<uint32_t>(result.verticies.size());
            for (int vert = 0; vert < 4; vert++)
            {
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                Vertex vertex{};
                vertex.position = glm::vec3(pos) + CUBE_VERTICES[cubeVertex];
                vertex.normal = CUBE_NORMALS[face];
                vertex.uv = getAtlasUV(face, CUBE_UVS[vert], blockType);
                vertex.color = {1, 1, 1};
                result.verticies.push_back(vertex);
            }

            for (size_t i : FACE_INDICES)
                result.indices.push_back(baseIndex + static_cast<uint32_t>(i));
        }
    }

    glm::ivec3 ChunkMeshWorkerPool::getDirection(int i)
    {
        return DIRECTIONS[i];
    }

    glm::vec2 ChunkMeshWorkerPool::getAtlasUV(int face, glm::vec2 uv, int blockType)
    {
        const float tileWidth = 16.0f / 32.0f;
        const float tileHeight = 16.0f / 48.0f;

        glm::vec2 v(uv.x, 1.0f - uv.y);

        int col = 0;
        int row = 1;

        if (blockType == 3)
        {
            col = 1;
            row = 0;
        }
        else if (blockType == 2)
        {
            col = 0;
            row = 2;
        }
        else
        {
            switch (face)
            {
            case 4:
                row = 0;
                break;
            case 5:
                row = 2;
                break;
            default:
                row = 1;
                break;
            }
        }

        glm::vec2 offset(col * tileWidth, row * tileHeight);
        return offset + glm::vec2(v.x * tileWidth, v.y * tileHeight);
    }
}
