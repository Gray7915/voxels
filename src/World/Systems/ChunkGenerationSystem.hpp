#pragma once
#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/ChunkGenWorkerPool.hpp"
#include "World/Area.hpp"

namespace lve
{
    class ChunkGenerationSystem
    {
    public:
        ChunkGenerationSystem(Area &area);
        ~ChunkGenerationSystem();

        void update(Area &area);
        void requestGeneration(glm::ivec3 coord);

    private:
        Area &area;
        ChunkGenWorkerPool genPool;
    };
}