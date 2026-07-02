#include "Chunk.hpp"
#include "ChunkRenderer.hpp"
#include <iostream>
#include "../Util/noise.hpp"
#include <algorithm>
#include "Area.hpp"

namespace lve
{
    Chunk::~Chunk() = default;
    Chunk::Chunk(LveDevice &lveDevice, glm::vec3 offset) : noise(4, 6, 0.0f)
    {
        Chunk::createChunk(lveDevice, offset);
        // std::cout << "created chunk at " << offset.x << offset.y << offset.z << '\n';
    }

    void Chunk::createChunk(LveDevice &lveDevice, glm::vec3 offset)
    {

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

        // createTrees();
        //  blocks[8][16][8] = 1;

        chunkModel = ChunkRenderer::mesh(this->blocks, lveDevice, offset);
 
        // gameObjects.insert({glm::ivec3(offset) / glm::ivec3(16, 32, 16), std::move(obj)});
        Area::chunks.emplace(glm::ivec3(offset) / glm::ivec3(16, 32, 16), std::unique_ptr<Chunk>(this));
    }

    void Chunk::createTrees()
    {
        for (int x = 0; x < width + 1; x++)
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
            }
    }
}
