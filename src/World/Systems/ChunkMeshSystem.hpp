#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkMeshWorkerPool.hpp"

namespace lve
{
    class ChunkMeshSystem
    {
    public:
        ChunkMeshSystem(Area &worldArea);
        ~ChunkMeshSystem();
        void Update(LveDevice &device);

    private:
        void tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk);
        Area &area;
        ChunkMeshWorkerPool meshPool;
    };
}
