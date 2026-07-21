#include <iostream>
#include <optional>

#include "World/Generation/ChunkMeshWorkerPool.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "App/TextureAtlas.hpp"

#include "Util/Direction.hpp"
#include "Util/Types.hpp"
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

    // static const glm::ivec3 DIRECTIONS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

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
                    {
                        continue;
                    }

                    if (job.voxelData.get(x, y, z) != 4)
                    {
                        emitBlock(job, result, ivec3(x, y, z), emittedFaces);
                    }
                    else
                    {
                        emitMesh(job, result, ivec3(x, y, z), emittedFaces);
                    }
                }
            }
        }
        //  std::cout<< "Chunk "<< job.chunkCoord.x << ","<< job.chunkCoord.y << ","<< job.chunkCoord.z<< " Faces: "<< emittedFaces<< " Vertices: "<< result.verticies.size()<< "\n";
        result.model = LveModel::createChunkModel(*job.device, result.verticies, result.indices, myPool);
        return result;
    }

    void ChunkMeshWorkerPool::emitBlock(MeshJob &job, MeshResult &result, ivec3 pos, u32 &emittedFaces)
    {
        int blockType = job.voxelData.get(pos.x, pos.y, pos.z);

        for (int face = 0; face < 6; face++)
        {
            glm::ivec3 n = pos + Math::DirectionByFaceInt(face);
            bool outOfChunk = n.x < 0 || n.x >= VoxelData::WIDTH || n.z < 0 || n.z >= VoxelData::DEPTH || n.y < 0 || n.y >= VoxelData::HEIGHT;

            bool neighborSolid = false;

            if (outOfChunk)
            {
                neighborSolid = getNeighborData(job, n);
            }
            else
            {
                neighborSolid = job.voxelData.get(n.x, n.y, n.z) != 0 && job.voxelData.get(n.x, n.y, n.z) != 4;
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
                int ao = calculateAO(pos, face, cubeVertex, job);

                // std::cout << "AO: " << ao << " value " << aoValues[ao] << "\n";

                vertex.ao = aoValues[ao];
                result.verticies.push_back(vertex);
            }

            for (size_t i : FACE_INDICES)
                result.indices.push_back(baseIndex + static_cast<uint32_t>(i));
        }
    }

    void ChunkMeshWorkerPool::emitMesh(MeshJob &job, MeshResult &result, glm::ivec3 pos, uint32_t &emittedFaces)
    {
        LveModel::Builder builder{};
        builder.loadModel("../models/TestFence.obj");
        uint32_t baseVertex = static_cast<uint32_t>(result.verticies.size());
        for (Vertex vert : builder.vertices)
        {
            // std::cout << "fence verts " << vert.position.x << " " << vert.position.y << " " << vert.position.z << '\n';

            vert.position = vert.position + glm::vec3(pos) + glm::vec3(0.5, 0, 0.5);

            // vert.uv = glm::vec2(0, 0);
            vert.color = glm::vec3(1.0);
            result.verticies.push_back(vert);
        }

        for (auto index : builder.indices)
        {
            result.indices.push_back(baseVertex + index);
        }
    }

    int ChunkMeshWorkerPool::getSign(glm::ivec3 tangent, glm::ivec3 vertex)
    {
        if (tangent.x != 0)
            return vertex.x ? 1 : -1;

        if (tangent.y != 0)
            return vertex.y ? 1 : -1;

        return vertex.z ? 1 : -1;
    }

    glm::ivec3 ChunkMeshWorkerPool::getFaceTangent1(int face)
    {
        if (face <= 1 || face >= 4)
            return {1, 0, 0};
        if (face >= 2 && face <= 3)
            return {0, 0, 1};
        return {};
    }

    glm::ivec3 ChunkMeshWorkerPool::getFaceTangent2(int face)
    {
        if (face <= 3)
            return {0, 1, 0};
        if (face <= 5)
            return {0, 0, 1};
        return {};
    }

    int ChunkMeshWorkerPool::calculateAO(glm::ivec3 pos, int face, int cubeVertex, MeshJob &job)
    {
        glm::ivec3 localVertex = glm::ivec3(CUBE_VERTICES[cubeVertex]);

        glm::ivec3 tangent1 = getFaceTangent1(face);
        glm::ivec3 tangent2 = getFaceTangent2(face);

        int sign1 = getSign(tangent1, localVertex);
        int sign2 = getSign(tangent2, localVertex);

        glm::ivec3 side1 = tangent1 * sign1;
        glm::ivec3 side2 = tangent2 * sign2;
        glm::ivec3 corner = side1 + side2;

        glm::ivec3 faceDir = Math::DirectionByFaceInt(face);

        int block1 = getSolid(pos + faceDir + side1, job);
        int block2 = getSolid(pos + faceDir + side2, job);
        int blockCorner = getSolid(pos + faceDir + corner, job);

        if (block1 + block2 == 2)
            return 0;

        return 3 - (block1 + block2 + blockCorner);
    }

    int ChunkMeshWorkerPool::getSolid(glm::ivec3 voxel, const MeshJob &job)
    {
        if (voxel.y < 0 || voxel.y >= VoxelData::HEIGHT)
            return 0;

        bool outOfX = voxel.x < 0 || voxel.x >= VoxelData::WIDTH;
        bool outOfZ = voxel.z < 0 || voxel.z >= VoxelData::DEPTH;

        if (outOfX || outOfZ)
        {
            glm::ivec3 clamped = voxel;

            if (outOfX && outOfZ)
            {

                clamped.x = glm::clamp(voxel.x, 0, VoxelData::WIDTH - 1);
                clamped.z = glm::clamp(voxel.z, -1, VoxelData::DEPTH);

                if (clamped.z < 0 || clamped.z >= VoxelData::DEPTH)
                {
                    clamped.x = glm::clamp(voxel.x, -1, VoxelData::WIDTH);
                    clamped.z = glm::clamp(voxel.z, 0, VoxelData::DEPTH - 1);
                }
            }
            else
            {
                clamped.x = glm::clamp(voxel.x, -1, VoxelData::WIDTH);
                clamped.z = glm::clamp(voxel.z, -1, VoxelData::DEPTH);
            }

            return getNeighborData(job, clamped) ? 1 : 0;
        }

        return job.voxelData.get(voxel.x, voxel.y, voxel.z) != 0 ? 1 : 0;
    }

    glm::vec2 ChunkMeshWorkerPool::getAtlasUV(int face, glm::vec2 uv, int blockType)
    {
        /*
        For models need to change this
        Probably have the texture put into the atlas at us the regular model uvs combined with
        the texture placement on the atlas to correctly texture the model
        atlasUV = regionOffset + (modelUV * regionSize)?
        */
        std::string requstTexture = BlockRegistry::Get().GetBlockByID(blockType)->get().name;
        // std::cout << requstTexture << '\n';

        const auto &region = TextureAtlas::Get().atlasRegions.find(requstTexture);
        if (region != TextureAtlas::Get().atlasRegions.end())
        {
            const auto &atlas = TextureAtlas::Get();

            float u = (region->second.x + uv.x * region->second.width) / static_cast<float>(atlas.atlasWidth);

            float v = (region->second.y + uv.y * region->second.height) / static_cast<float>(atlas.atlasHeight);

            return {u, v};
        }
        return uv;
    }

    glm::vec2 ChunkMeshWorkerPool::getModelAtlasUV(glm::vec2 modelUV, const std::string &textureName)
    {
        const auto &atlas = TextureAtlas::Get();
        const auto &region = atlas.atlasRegions.at(textureName);

        return {
            (region.x + modelUV.x * region.width) / atlas.atlasWidth,
            (region.y + modelUV.y * region.height) / atlas.atlasHeight};
    }

    bool ChunkMeshWorkerPool::getNeighborData(const MeshJob &job, glm::ivec3 v)
    {
        // corners — check before edges
        if (v.x == 16 && v.z == 16)
            return job.neighborVoxelData.get(16, v.y, 0) != 0;
        if (v.x == 16 && v.z == -1)
            return job.neighborVoxelData.get(16, v.y, 3) != 0;
        if (v.x == -1 && v.z == 16)
            return job.neighborVoxelData.get(16, v.y, 1) != 0;
        if (v.x == -1 && v.z == -1)
            return job.neighborVoxelData.get(16, v.y, 2) != 0;

        // edges
        if (v.x == 16)
            return job.neighborVoxelData.get(v.z, v.y, 0) != 0;
        if (v.z == 16)
            return job.neighborVoxelData.get(v.x, v.y, 1) != 0;
        if (v.x == -1)
            return job.neighborVoxelData.get(v.z, v.y, 2) != 0;
        if (v.z == -1)
            return job.neighborVoxelData.get(v.x, v.y, 3) != 0;

        return false;
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
