#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "World/Generation/ChunkMeshWorkerPool.hpp"

#include "World/NeighborVoxelInfo.hpp"
#include "World/VoxelData.hpp"
#include "Rendering/Core/ChunkStagingPool.hpp"

namespace lve
{
    class Area;
    class ChunkMeshSystem {
      public:
        ChunkMeshSystem(Area &worldArea, LveDevice &device, ChunkStagingPool &stagingPool);
        ~ChunkMeshSystem();
        void Update(LveDevice &device, int frameIndex);

      private:
        void tryQueueForMeshing(glm::ivec3 coord, Chunk &chunk, LveDevice &lveDevice, NeighborVoxelInfo neighborVoxelInfo);
        void getNeighborChunkInfo(glm::ivec3 chunkDir, VoxelData &chunkData, NeighborVoxelInfo &neighborChunkInfo);
        Area &area;
        LveDevice &device;
        ChunkMeshWorkerPool meshPool{device, 4};
        ChunkStagingPool &stagingPool;
    };
} // namespace lve
