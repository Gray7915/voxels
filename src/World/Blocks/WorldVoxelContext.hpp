#pragma once
#include "World/Blocks/VoxelEmitter.hpp"
#include "Util/Types.hpp"
#include "World/VoxelData.hpp"
#include "Util/ThreadSafeQueue.hpp"

namespace Blocks
{
    class WorldVoxelContext : public IVoxelContext {
      public:
        explicit WorldVoxelContext(const lve::MeshJob &job) : job(job) {}

        Voxel voxelAt(ivec3 p) const override;
        bool cullFace(ivec3 pos, int face) const override;
        float ao(ivec3 pos, int face, size_t cubeVertex) const override;

      private:
        int calculateAO(ivec3 pos, int face, usize cubeVertex) const;
        int isSolid(ivec3 p) const;
        static int getSign(ivec3 tangent, ivec3 vertex);
        static ivec3 faceTangent1(int face);
        static ivec3 faceTangent2(int face);

        static constexpr float aoValues[4] = {0.1f, 0.34f, 0.65f, 1.f};

        const lve::MeshJob &job;
    };
} // namespace Blocks