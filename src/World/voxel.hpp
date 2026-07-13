#pragma once

#include <cstdint>
#include <bit>

inline constexpr int BLOCK_DIR_NORTH = 0x0;
inline constexpr int BLOCK_DIR_WEST = 0x1;
inline constexpr int BLOCK_DIR_SOUTH = 0x2;
inline constexpr int BLOCK_DIR_EAST = 0x3;
inline constexpr int BLOCK_DIR_UP = 0x4;
inline constexpr int BLOCK_DIR_DOWN = 0x5;

struct blockState
{
    uint8_t rotation : 3;  // three bits for rotation
    uint8_t segment : 5;   // bits of a multiblock block
    uint8_t usagebits : 8; // growth stage ect ?
};

static_assert(sizeof(blockState) == 2);
static_assert(alignof(blockState) == 1);
static_assert(sizeof(blockState) == sizeof(uint16_t));

inline constexpr blockState int2blockstate(uint16_t i) {
    return std::bit_cast<blockState>(i);
}

inline constexpr uint16_t blockstate2int(blockState b) {
    return std::bit_cast<uint16_t>(b);
}
