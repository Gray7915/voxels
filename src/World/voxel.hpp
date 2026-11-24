#pragma once

#include <cstdint>
#include <bit>
#include "Util/Types.hpp"

inline constexpr int BLOCK_DIR_NORTH = 0x0;
inline constexpr int BLOCK_DIR_WEST = 0x1;
inline constexpr int BLOCK_DIR_SOUTH = 0x2;
inline constexpr int BLOCK_DIR_EAST = 0x3;
inline constexpr int BLOCK_DIR_UP = 0x4;
inline constexpr int BLOCK_DIR_DOWN = 0x5;

struct blockState {
    u8 rotation : 3;  // three bits for rotation
    u8 segment : 5;   // bits of a multiblock block
    u8 usagebits : 8; // growth stage ect ?
};

static_assert(alignof(blockState) == 1);
static_assert(sizeof(blockState) == sizeof(u16));

inline constexpr blockState IntToBlockState(u16 i) { return std::bit_cast<blockState>(i); }

inline constexpr u16 BlockStateTwoInt(blockState b) { return std::bit_cast<u16>(b); }

struct Voxel {
    u16 blockID;
    blockState state;
};

static_assert(sizeof(Voxel) == 4);
