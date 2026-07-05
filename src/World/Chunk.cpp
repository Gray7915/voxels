#include "Chunk.hpp"
#include "ChunkRenderer.hpp"
#include <iostream>
#include "../Util/noise.hpp"
#include <algorithm>
#include "Area.hpp"

namespace lve
{
    Chunk::~Chunk() = default;
    Chunk::Chunk(LveDevice &lveDevice, glm::vec3 offset)
    {
        Chunk::createChunk(lveDevice, offset);
        // std::cout << "created chunk at " << offset.x << offset.y << offset.z << '\n';
    }

    void Chunk::createChunk(LveDevice &lveDevice, glm::vec3 offset)
    {
        /*
         for (int x = 0; x < width + 1; x++)
                    for (int z = 0; z < width + 1; z++)
                    {
                        glm::vec2 worldPos = glm::vec2(x + offset.x, z + offset.z);

                        float heightValue = noise.sample(worldPos * 0.009f);

                        heightValue = (heightValue + 1.0f) * 0.5f;
                        // std::cout << "height noise val " << heightValue << "\n";

                        int surfaceHeight = (int)(heightValue * (height - 1));
                        int stoneHeight = surfaceHeight - 3;

                        for (int y = 0; y < height; y++)
                        {
                            if (y > surfaceHeight)
                                blocks[x][y][z] = 0;
                            else if (y == surfaceHeight)
                                blocks[x][y][z] = 1;
                            else if (y < surfaceHeight && y >= stoneHeight)
                                blocks[x][y][z] = 2;
                            else if (y < stoneHeight)
                            {
                                blocks[x][y][z] = 3;
                            }
                        }
                    }

                chunkModel = ChunkRenderer::mesh(this->blocks, lveDevice, offset);
                this->offset = offset;
                this->scale = {1.f, 1.f, 1.f};
                this->rotation = glm::vec3(0.0f);
        */
    }

    void Chunk::createTrees()
    {
        /*  for (int x = 0; x < width + 1; x++)
            for (int z = 0; z < width + 1; z++)
            {
                glm::vec2 worldPos = glm::vec2(x + offset.x, z + offset.z);

                float heightValue = noise.sample(worldPos * 0.03f);
                heightValue = (heightValue + 1.0f) * 0.5f;
                float treeNoise = noise.sample(worldPos * 0.3f);
                bool placeTree = treeNoise > 0.24f;

                if (!placeTree)
                    continue;

                int trunkHeight = 0;
                for (int y = 0; y < height && trunkHeight < 5; y++)
                {
                    if (blocks[x][y][z] == 0)
                    {
                        blocks[x][y][z] = 3;
                        trunkHeight++;
                    }
                }
            }*/
    }

    glm::mat4 Chunk::mat4()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {offset.x, offset.y, offset.z, 1.0f}};
    }

    glm::mat3 Chunk::normalMatrix()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
            {
                invScale.x * (c1 * c3 + s1 * s2 * s3),
                invScale.x * (c2 * s3),
                invScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                invScale.y * (c3 * s1 * s2 - c1 * s3),
                invScale.y * (c2 * c3),
                invScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                invScale.z * (c2 * s1),
                invScale.z * (-s2),
                invScale.z * (c1 * c2),
            },
        };
    }
}
