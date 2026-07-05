#pragma once
#include <cstdint>
#include <vector>

namespace lve
{
    using BlockID = uint16_t;
    class VoxelData
    {
    public:
        static constexpr int WIDTH = 16;
        static constexpr int HEIGHT = 128;
        static constexpr int DEPTH = 16;
        static constexpr int VOLUME = WIDTH * HEIGHT * DEPTH;

        void allocate(BlockID fillValue = 0)
        {
            blocks.assign(VOLUME, fillValue);
        }

        bool isGenerated() const
        {
            return !blocks.empty();
        }

        BlockID get(int x, int y, int z) const
        {
            return blocks[index(x, y, z)];
        }

        void set(int x, int y, int z, BlockID id)
        {
            blocks[index(x, y, z)] = id;
        }

    private:
        static int index(int x, int y, int z)
        {
            return x + WIDTH * (z + DEPTH * y);
        }
        std::vector<BlockID> blocks;
    };
}