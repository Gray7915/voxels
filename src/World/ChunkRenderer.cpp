#include "Chunk.hpp"
#include "../lve_model.hpp"
#include "ChunkRenderer.hpp"
#include <iostream>

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

    void ChunkRenderer::emit_tile(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, glm::ivec3 pos, int blocks[16][32][16], glm::vec3 worldOffset)
    {
        // std::cout << "start pos " << pos.x << " " << pos.y << " " << pos.z << '\n';
        const auto uv_unit = glm::vec2(1.0f) / glm::vec2(16.0f);

        for (int face = 0; face < 6; face++)
        {
            glm::ivec3 n = pos + getDirection(face);

            // std::cout << "pos " << n.x << " " << n.y << " " << n.z << '\n';
            bool visible =
                n.x < 0 || n.y < 0 || n.z < 0 ||
                n.x >= 16 || n.y >= 32 || n.z >= 16 ||
                blocks[n.x][n.y][n.z] == 0;

            if (!visible)
                continue;

            const size_t offset = vertices.size();
            glm::vec2 uv_size = glm::vec2(1, 16 - 1 - 1) * uv_unit;
            for (int vert = 0; vert < 4; vert++)
            {
                Vertex vertex;
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                vertex.position = glm::vec3(pos) + CUBE_VERTICES[cubeVertex];
                vertex.normal = CUBE_NORMALS[face];
                vertex.uv = getAtlasUV(face, CUBE_UVS[vert]);
                vertex.color = {1, 1, 1};
                glm::ivec3 chunkOrigin = glm::ivec3(worldOffset) * glm::ivec3(16, 32, 16);

                vertices.push_back(vertex);
            }

            glm::ivec3 blockWorldPosition = pos + (glm::ivec3(worldOffset) * glm::ivec3(16, 32, 16));

            //std::cout << "block pos set " << blockWorldPosition.x << " " << blockWorldPosition.y << " " << blockWorldPosition.z << '\n';

            for (size_t i : FACE_INDICES)
            {
                indices.push_back(offset + i);
            }
        }
        // std::cout << "finished block" << '\n';
    }

    LveGameObject ChunkRenderer::mesh(int blocks[16][32][16], LveDevice &lveDevice, glm::vec3 offset)
    {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        Vertex pos;
        for (int x = 0; x < Chunk::width; x++)
        {
            for (int y = 0; y < Chunk::height; y++)
            {
                for (int z = 0; z < Chunk::width; z++)
                {
                    if (blocks[x][y][z] == 0)
                        continue;

                    pos.position = glm::vec3(x, y, z) + offset;
                    emit_tile(vertices, indices, glm::ivec3(x, y, z), blocks, offset);
                    // vertices.push_back(pos);
                }
            }
        }

        // std::cout << "finished making mesh" << '\n';
        LveGameObject firstChunk = LveGameObject::createGameObject();
        firstChunk.model = LveModel::createChunkModel(lveDevice, vertices, indices);
        // std::cout << "loaded model" << '\n';

        firstChunk.transform.translation = offset;
        firstChunk.transform.scale = {1.f, 1.f, 1.f};
        // std::cout << "returned chunk" << '\n';
        return firstChunk;
    }

    glm::vec2 ChunkRenderer::getAtlasUV(int face, glm::vec2 uv)
    {
        float tileHeight = 1.0f / 3.0f;

        float offsetY = 0.0f;

        switch (face)
        {
        case 4:
            offsetY = 2.0f * tileHeight;
            break;

        case 5:
            offsetY = 0.0f;
            break;

        default:
            offsetY = 1.0f * tileHeight;
            break;
        }

        return glm::vec2(
            uv.x,
            uv.y * tileHeight + offsetY);
    }
}
