#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <assert.h>

namespace lve
{
    using BlockID = uint16_t;
    class NeighborVoxelInfo
    {
    public:
        static constexpr int WIDTH = 17;
        static constexpr int HEIGHT = 128;
        static constexpr int DEPTH = 4;
        static constexpr int VOLUME = WIDTH * HEIGHT * DEPTH;

        void allocate(Voxel fillValue = Voxel{0, {0, 0, 0}})
        {
            blocks.assign(VOLUME, fillValue);
        }

        bool isGenerated() const
        {
            return !blocks.empty();
        }

        BlockID get(int x, int y, int z) const
        {
            assert(x >= 0 && x < WIDTH);
            assert(y >= 0 && y < HEIGHT);
            assert(z >= 0 && z < DEPTH);

            int i = blocks[index(x, y, z)].blockID;
            return i;
        }

        Voxel getVoxel(int x, int y, int z) const
        {
            assert(x >= 0 && x < WIDTH);
            assert(y >= 0 && y < HEIGHT);
            assert(z >= 0 && z < DEPTH);

            int i = blocks[index(x, y, z)].blockID;
            return blocks[index(x, y, z)];
        }

        void set(int x, int y, int z, BlockID id)
        {
            assert(!blocks.empty());
            assert(index(x, y, z) < blocks.size());
            blocks[index(x, y, z)].blockID = id;
        }

    private:
        static int index(int x, int y, int z)
        {
            return x + WIDTH * (z + DEPTH * y);
        }
        std::vector<Voxel> blocks;
    };
}
