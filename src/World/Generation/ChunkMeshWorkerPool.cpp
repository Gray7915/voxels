#include <vector>
#include <thread>
#include <atomic>

#include "Util/ThreadSafeQueue.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    static const size_t UNIQUE_INDICES[] = {0, 1, 2, 5};

    // indices into emitted vertices which make up the two faces for a cube face
    static const size_t FACE_INDICES[] = {0, 1, 2, 0, 2, 3};

    static const size_t CUBE_INDICES[] = {
        4, 7, 6, 4, 6, 5, // (south (+z))
        3, 0, 1, 3, 1, 2, // (north (-z))
        7, 3, 2, 7, 2, 6, // (east  (+x))
        0, 4, 5, 0, 5, 1, // (west  (-x))
        2, 1, 5, 2, 5, 6, // (up    (+y))
        0, 3, 7, 0, 7, 4  // (down  (-y))
    };

    static const glm::vec3 CUBE_VERTICES[] = {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(1, 1, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 1, 1),
        glm::vec3(1, 1, 1),
        glm::vec3(1, 0, 1)};

    static const glm::vec3 CUBE_NORMALS[] = {
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1),
        glm::vec3(1, 0, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0),
    };

    static const glm::vec2 CUBE_UVS[] = {
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec2(1, 1),
        glm::vec2(0, 1),
    };

    class ChunkMeshWorkerPool
    {
        void workerPool()
        {
            while (running)
            {
                MeshJob job = jobQueue.wait_and_pop();
                resultQueue.push(generateMesh(job));
            }
        }

        MeshResult generateMesh(MeshJob &job)
        {
            MeshResult result{};
            for (int x = 0; x < VoxelData::WIDTH; x++)
            {
                for (int z = 0; z < VoxelData::DEPTH; z++)
                {
                    for (int y = 0; y < VoxelData::HEIGHT; y++)
                    {
                        emitBlock(job, result, glm::ivec3(x, y, z));
                    }
                }
            }
            return result;
        }

        void emitBlock(MeshJob &job, MeshResult &result, glm::ivec3 pos)
        {
            const auto uv_unit = glm::vec2(1.0f) / glm::vec2(16.0f);

            for (int face = 0; face < 6; face++)
            {
                glm::ivec3 n = pos + getDirection(face);
                bool visible = n.x < 0 || n.y < 0 || n.z < 0 || n.x >= 16 || n.y >= 128 || n.z >= 16 || job.chunk->voxelData.get(n.x, n.y, n.z) == 0 || job.chunk->voxelData.get(n.x, n.y, n.z) == 4;

                if (!visible)
                    continue;

                const size_t offset = result.verticies.size();
                glm::vec2 uv_size = glm::vec2(1, 16 - 1 - 1) * uv_unit;
                for (int vert = 0; vert < 4; vert++)
                {
                    Vertex vertex;
                    size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                    vertex.position = glm::vec3(pos) + CUBE_VERTICES[cubeVertex];
                    vertex.normal = CUBE_NORMALS[face];
                    vertex.uv = getAtlasUV(face, CUBE_UVS[vert], job.chunk->voxelData.get(pos.x, pos.y, pos.z));
                    vertex.color = {1, 1, 1};
                    glm::ivec3 chunkOrigin = glm::ivec3(job.worldOffset) * glm::ivec3(16, 128, 16);

                    result.verticies.push_back(vertex);
                }
            }
        }

        static glm::ivec3 getDirection(int i)
        {
            return ((glm::ivec3[]){
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0),
                glm::ivec3(0, -1, 0)})[i];
        }

        glm::vec2 getAtlasUV(int face, glm::vec2 uv, int blockType)
        {
            const float tileWidth = 16.0f / 32.0f;
            const float tileHeight = 16.0f / 48.0f;

            glm::vec2 v(uv.x, 1.0f - uv.y); // Vulkan-style flip applied once

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

    public:
        std::vector<std::thread> workers;
        ThreadSafeQueue<MeshJob> jobQueue;
        ThreadSafeQueue<MeshResult> resultQueue;
        std::atomic<bool> running{true};

        bool tryGetResult(MeshResult &out)
        {
            return resultQueue.try_pop(out);
        }
    };
}