#include "Area.hpp"
#include "../Rendering/Core/lve_device.hpp"
#include <iostream>
#include "Util/math.hpp"

namespace lve
{
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> Area::chunks;

    Area::~Area() = default;
    Area::Area(LveDevice &lveDevice, glm::vec3 offset)
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
                glm::ivec3 worldPos = chunkCoord * glm::ivec3(16, 1, 16);

                chunks.emplace(chunkCoord, std::make_unique<Chunk>(lveDevice, worldPos));
            }
        }
        // std::cout << "chunks made " << i << '\n';
    }

    void Area::tick(LveDevice &lveDevice, glm::vec3 center, uint32_t currentFrameIndex)
    {
        glm::ivec3 c = glm::ivec3(center) / glm::ivec3(16, 32, 16);
        for (auto it = chunks.begin(); it != chunks.end();)
        {
            const glm::ivec3 &coord = it->first;
            if (coord.x < c.x - MinMaxOffset || coord.x > c.x + MinMaxOffset ||
                coord.z < c.z - MinMaxOffset || coord.z > c.z + MinMaxOffset)
            {
                // Move the chunk out instead of destroying it now.
                // Its destructor (which frees Vulkan buffers) runs later,
                // once the GPU is confirmed done with this frame slot.
                auto chunkPtr = std::shared_ptr<Chunk>(std::move(it->second));
                lveDevice.queueDeletion([chunk = chunkPtr]() {}, currentFrameIndex);
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
                glm::ivec3 chunkCoord = glm::ivec3(x, 0, z);
                if (chunks.find(chunkCoord) == chunks.end())
                {
                    glm::ivec3 worldPos = chunkCoord * glm::ivec3(16, 32, 16);
                    chunks.emplace(chunkCoord, std::make_unique<Chunk>(lveDevice, worldPos));
                }
            }
        }
    }

    bool Area::isBlockSolid(glm::vec3 worldBlockPos)
    {
        glm::vec3 chunkId = WorldToChunkId(worldBlockPos);
        // std::cout << "block hit chunk ID " << " " << chunkId.x << " " << chunkId.y << " " << chunkId.z << '\n';
        auto block = chunks.find(chunkId);
        if (block == chunks.end() || !block->second)
            return false;
        glm::ivec3 arrayPos = WorldToChunkArray(worldBlockPos);
        // std::cout << "block hit array " << " " << arrayPos.x << " " << arrayPos.y << " " << arrayPos.z << '\n';
        return block->second->blocks[arrayPos.x][arrayPos.y][arrayPos.z] != 0;
    }

    void Area::reMeshChunk(glm::ivec3 chunkPosition)
    {
    }

}
