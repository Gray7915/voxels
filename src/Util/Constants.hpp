#pragma once

#include "Util/Types.hpp"

using BlockId = u16;

static constexpr u16 CHUNK_WIDTH = 16;
static constexpr u16 CHUNK_HEIGHT = 128;
static constexpr u16 CHUNK_DEPTH = 16;
static constexpr u16 CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;