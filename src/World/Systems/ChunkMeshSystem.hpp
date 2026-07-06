#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkMeshWorkerPool.hpp"

namespace lve
{
    class ChunkMeshSystem
    {
    public:
        ChunkMeshSystem(Area &worldArea, LveDevice &device);
        ~ChunkMeshSystem();
        void Update(LveDevice &device, int frameIndex);

    private:
        void tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk, LveDevice &lveDevice);
        Area &area;
        LveDevice &device;
        ChunkMeshWorkerPool meshPool{device};
    };
}
