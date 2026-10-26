#include <memory>

#include "World/Blocks/VoxelEmitter.hpp"
#include "Rendering/Core/lve_model.hpp"

namespace Blocks
{
    class IconVoxelContext : public IVoxelContext {
      public:
        explicit IconVoxelContext(int blockId) : id(blockId) {}

        Voxel voxelAt(ivec3 p) const override {
            Voxel v{}; // assumes default Voxel is air
            if (p == ivec3(0))
                v.blockID = id;
            return v;
        }
        vec3 origin(ivec3) const override { return vec3(-0.5f); }
        vec3 color() const override { return {0.1f, 0.1f, 0.1f}; }

      private:
        int id;
    };
} // namespace Blocks
