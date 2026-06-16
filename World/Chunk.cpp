#include "Chunk.hpp"
#include "ChunkRenderer.hpp"
#include <iostream>
#include "../Util/noise.hpp"
#include <algorithm>

namespace lve
{
    Chunk::~Chunk() = default;
    Chunk::Chunk(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 offset) : noise(4, 6, 0.0f)
    {
        Chunk::createChunk(gameObjects, lveDevice, offset);
        std::cout << "created chunk at " << offset.x << offset.y << offset.z << '\n';
    }

    void Chunk::createChunk(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 offset)
    {
        for (int x = 0; x < width; x++)
            for (int z = 0; z < width; z++)
            {
                glm::vec2 worldPos = glm::vec2(x + offset.x, z + offset.z);

                float heightValue = noise.sample(worldPos * 0.03f);

                heightValue = (heightValue + 1.0f) * 0.5f;

                int surfaceHeight = (int)(heightValue * (height - 1));
                               
                for (int y = 0; y < height; y++)
                {
                    if (y < surfaceHeight)
                        blocks[x][y][z] = 0;
                    else if (y >= surfaceHeight)
                        blocks[x][y][z] = 1;
                }
            }

        auto obj = ChunkRenderer::mesh(this->blocks, lveDevice, offset);

        gameObjects.insert({obj.transform.translation, std::move(obj)});
    }
}
