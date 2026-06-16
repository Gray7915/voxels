#include "Area.hpp"
#include "../lve_device.hpp"
#include <iostream>

namespace lve
{
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> Area::chunks;
    
    Area::~Area() = default;
    Area::Area(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 offset)
    {

        int i = 0;
        for (int x = offset.x - MinMaxOffset; x <= offset.x + MinMaxOffset; x++)
        {
            for (int z = offset.z - MinMaxOffset; z <= offset.z + MinMaxOffset; z++)
            {
                i++;
                // std::cout << "loop counters " << " " << x << " " << z << '\n';

                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                // std::cout << "chunk coordinate " << " " << chunkCoord.x << " " << chunkCoord.y << " " << chunkCoord.z << '\n';
                glm::ivec3 worldPos = chunkCoord * glm::ivec3(16, 32, 16);

                chunks.emplace(chunkCoord, std::make_unique<Chunk>(gameObjects, lveDevice, worldPos));
            }
        }
        // std::cout << "chunks made " << i << '\n';
    }

    void Area::tick(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 center)
    {
        glm::ivec3 c = glm::ivec3(center) / glm::ivec3(16, 32, 16);

        for (auto it = chunks.begin(); it != chunks.end();)
        {
            const glm::ivec3 &coord = it->first; // already chunk coords
            if (coord.x < c.x - MinMaxOffset || coord.x > c.x + MinMaxOffset ||
                coord.z < c.z - MinMaxOffset || coord.z > c.z + MinMaxOffset)
            {
                it = chunks.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (int x = c.x - MinMaxOffset; x <= c.x + MinMaxOffset; x++)
        {
            for (int z = c.z - MinMaxOffset; z <= c.z + MinMaxOffset; z++)
            {
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z); // / glm::ivec3(16, 32, 16);
                // std::cout << "chunk coordinate " << " " << chunkCoord.x << " " << chunkCoord.y << " " << chunkCoord.z << '\n';

                if (chunks.find(chunkCoord) == chunks.end())
                {
                    // std::cout << "creating chunk\n";
                    glm::ivec3 worldPos = chunkCoord * glm::ivec3(16, 32, 16);
                    // std::cout << "chunk world pos " << " " << worldPos.x << " " << worldPos.y << " " << worldPos.z << '\n';

                    chunks.emplace(chunkCoord, std::make_unique<Chunk>(gameObjects, lveDevice, worldPos));
                }
            }
        }
    }

}
