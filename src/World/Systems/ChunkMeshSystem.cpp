#include "ChunkMeshSystem.hpp"
#include "World/Area.hpp"
#include "World/ChunkRenderer.hpp"

namespace lve
{
    void ChunkMeshSystem::Update(LveDevice &device, uint32_t currentFrameIndex)
    {
        for (auto &[pos, chunk] : Area::chunks)
        {
            if (chunk->dirty && chunk->chunkModel != nullptr)
            {
                auto modelPtr = std::move(chunk->chunkModel);
                device.queueDeletion([model = modelPtr]() {}, currentFrameIndex);
                chunk->chunkModel = ChunkRenderer::mesh(chunk->blocks, device, {0, 0, 0});
                chunk->dirty = false;
            }
        }
    }
}
