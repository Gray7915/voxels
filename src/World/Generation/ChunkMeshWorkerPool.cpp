#include "World/Generation/ChunkMeshWorkerPool.hpp"
#include "Rendering/Core/lve_device.hpp"
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

    ChunkMeshWorkerPool::ChunkMeshWorkerPool(LveDevice &device, size_t threadCount) : device(device)
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
        myPool = device.createTransientCommandPool();
        MeshJob job;
        while (jobQueue.wait_and_pop(job))
        {
            resultQueue.push(generateMesh(job));
        }

        device.destroyCommandPool(myPool);
    }

    MeshResult ChunkMeshWorkerPool::generateMesh(MeshJob &job)
    {
        MeshResult result{};
        result.chunkCoord = job.chunkCoord;
        uint32_t emittedFaces = 0;
        // std::cout << "chunk coord in gen mesh " << job.chunkCoord.x << ", " << job.chunkCoord.y << ", " << job.chunkCoord.z << '\n';
        // std::cout << "world offset in gen mesh " << job.worldOffset.x << ", " << job.worldOffset.y << ", " << job.worldOffset.z << '\n';

        for (int x = 0; x < VoxelData::WIDTH; x++)
        {
            for (int z = 0; z < VoxelData::DEPTH; z++)
            {
                for (int y = 0; y < VoxelData::HEIGHT; y++)
                {
                    if (job.voxelData.get(x, y, z) == 0)
                        continue; // skip air — was missing entirely before

                    emitBlock(job, result, glm::ivec3(x, y, z), emittedFaces);
                }
            }
        }
        //  std::cout<< "Chunk "<< job.chunkCoord.x << ","<< job.chunkCoord.y << ","<< job.chunkCoord.z<< " Faces: "<< emittedFaces<< " Vertices: "<< result.verticies.size()<< "\n";
        result.model = LveModel::createChunkModel(*job.device, result.verticies, result.indices, myPool);
        return result;
    }

    void ChunkMeshWorkerPool::emitBlock(MeshJob &job, MeshResult &result, glm::ivec3 pos, uint32_t &emittedFaces)
    {
        int blockType = job.voxelData.get(pos.x, pos.y, pos.z);

        for (int face = 0; face < 6; face++)
        {
            glm::ivec3 n = pos + getDirection(face);
            bool thingie = !getNeighborData(job, n);
            // std::cout << thingie << "hopefully this works?" << '\n';
            bool outOfChunk = n.x < 0 || n.x >= VoxelData::WIDTH || n.z < 0 || n.z >= VoxelData::DEPTH || n.y < 0 || n.y >= VoxelData::HEIGHT;

            bool neighborSolid = false;

            if (outOfChunk)
            {
                neighborSolid = getNeighborData(job, n);
            }
            else
            {
                neighborSolid = job.voxelData.get(n.x, n.y, n.z) != 0;
            }

            bool visible = !neighborSolid;

            if (!visible)
                continue;
            emittedFaces++;
            const uint32_t baseIndex = static_cast<uint32_t>(result.verticies.size());
            for (int vert = 0; vert < 4; vert++)
            {
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                Vertex vertex{};
                vertex.position = glm::vec3(pos) + CUBE_VERTICES[cubeVertex];
                vertex.normal = CUBE_NORMALS[face];
                vertex.uv = getAtlasUV(face, CUBE_UVS[vert], blockType);
                vertex.color = {1, 1, 1};
                int ao = calculateAO(pos, face, vert, job);

                // std::cout << "AO: " << ao << " value " << aoValues[ao] << "\n";

                vertex.ao = aoValues[ao];
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

    glm::ivec3 ChunkMeshWorkerPool::getFaceTangent1(int face)
    {
        switch (face)
        {
        case 0:
            return {1, 0, 0};
        case 1:
            return {-1, 0, 0};
        case 2:
            return {0, 0, -1};
        case 3:
            return {0, 0, 1};
        case 4:
            return {1, 0, 0};
        case 5:
            return {1, 0, 0};
        }

        return {};
    }

    glm::ivec3 ChunkMeshWorkerPool::getFaceTangent2(int face)
    {
        switch (face)
        {
        case 0:
            return {0, 1, 0};
        case 1:
            return {0, 1, 0};
        case 2:
            return {0, 1, 0};
        case 3:
            return {0, 1, 0};
        case 4:
            return {0, 0, 1};
        case 5:
            return {0, 0, -1};
        }

        return {};
    }

    int ChunkMeshWorkerPool::calculateAO(glm::ivec3 pos, int face, int vertexIndex, MeshJob &job)
    {
        int cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vertexIndex]];

        glm::ivec3 localVertex = glm::ivec3(CUBE_VERTICES[cubeVertex]);

        glm::ivec3 tangent1 = getFaceTangent1(face);
        glm::ivec3 tangent2 = getFaceTangent2(face);

        int sign1 = getSign(tangent1, localVertex);
        int sign2 = getSign(tangent2, localVertex);

        glm::ivec3 side1 = tangent1 * sign1;
        glm::ivec3 side2 = tangent2 * sign2;
        glm::ivec3 corner = side1 + side2;

        glm::ivec3 faceDir = getDirection(face); // NEW: step into the layer the face actually sits in

        auto solid = [&](glm::ivec3 p) -> int
        {
            // y is never a chunk border — just world top/bottom, which is air.
            if (p.y < 0 || p.y >= VoxelData::HEIGHT)
                return 0;

            bool outOfX = p.x < 0 || p.x >= VoxelData::WIDTH;
            bool outOfZ = p.z < 0 || p.z >= VoxelData::DEPTH;

            if (outOfX || outOfZ)
            {
                return getNeighborData(job, p) ? 1 : 0;
            }

            return job.voxelData.get(p.x, p.y, p.z) > 1 ? 1 : 0;
        };

        int block1 = solid(pos + faceDir + side1);
        int block2 = solid(pos + faceDir + side2);
        int blockCorner = solid(pos + faceDir + corner);

        if (block1 + block2 == 2)
            return 0;

        return 3 - (block1 + block2 + blockCorner);
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

    bool ChunkMeshWorkerPool::getNeighborData(MeshJob &job, glm::ivec3 chunkVoxel)
    {
        // x = 16 -> z = 0
        if (chunkVoxel.x == 16 && chunkVoxel.z == 16)
        {
            return job.neighborVoxelData.get(16, chunkVoxel.y, 0) != 0;
        }
        else if (chunkVoxel.x == -1 && chunkVoxel.z == 16)
        {
            return job.neighborVoxelData.get(16, chunkVoxel.y, 1) != 0;
        }
        else if (chunkVoxel.x == -1 && chunkVoxel.z == -1)
        {
            return job.neighborVoxelData.get(16, chunkVoxel.y, 2) != 0;
        }
        else if (chunkVoxel.x == 16 && chunkVoxel.z == -1)
        {
            return job.neighborVoxelData.get(16, chunkVoxel.y, 3) != 0;
        }
        else if (chunkVoxel.x == 16)
        {
            // std::cout<< " neighbor voxel data " << job.neighborVoxelData.get(chunkVoxel.z, chunkVoxel.y, 0) << '\n';
            return job.neighborVoxelData.get(chunkVoxel.z, chunkVoxel.y, 0) != 0;
        }
        // z = 16 -> z = 1
        else if (chunkVoxel.z == 16)
        {
            return job.neighborVoxelData.get(chunkVoxel.x, chunkVoxel.y, 1) != 0;
        }
        // x = -1 -> z = 2
        else if (chunkVoxel.x == -1)
        {
            return job.neighborVoxelData.get(chunkVoxel.z, chunkVoxel.y, 2) != 0;
        }
        // z = -1 -> z = 3
        else if (chunkVoxel.z == -1)
        {
            return job.neighborVoxelData.get(chunkVoxel.x, chunkVoxel.y, 3) != 0;
        }

        return false;
    }

}
