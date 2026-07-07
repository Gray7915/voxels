#pragma once
#include "Util/ThreadSafeQueue.hpp"
#include "World/Generation/ChunkGenWorkerPool.hpp"

namespace lve
{
    class Area;
    class ChunkGenerationSystem
    {
    public:
        ChunkGenerationSystem(Area &area);
        ~ChunkGenerationSystem();

        void update();
        void requestGeneration(glm::ivec3 coord);

    private:
        Area &area;
        ChunkGenWorkerPool genPool;
    };
}
