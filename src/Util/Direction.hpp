#pragma once
#include <glm/glm.hpp>
#include <array>
#include <cassert>

#include "Util/Types.hpp"

namespace Math
{
    enum class Direction
    {
        NORTH,
        SOUTH,
        EAST,
        WEST,
        UP,
        DOWN
    };
    constexpr std::array<ivec3, 4> HorizontalCardinal = {{
        {0, 0, 1},
        {0, 0, -1},
        {1, 0, 0},
        {-1, 0, 0},
    }};

    constexpr std::array<ivec3, 6> CardinalDirections = {{
        {0, 0, 1},
        {0, 0, -1},
        {1, 0, 0},
        {-1, 0, 0},
        {0, 1, 0},
        {0, -1, 0},
    }};

    constexpr std::array<ivec3, 4> HorizontalDiagonals = {{
        {1, 0, -1},
        {1, 0, 1},
        {-1, 0, -1},
        {-1, 0, 1},
    }};

    constexpr std::array<ivec3, 10> AllHorizontalDirections = {{
        {0, 0, 1},
        {0, 0, -1},
        {1, 0, 0},
        {-1, 0, 0},
        {1, 0, -1},
        {1, 0, 1},
        {-1, 0, -1},
        {-1, 0, 1},
    }};

    constexpr ivec3 DirectionByFaceInt(int face)
    {
        assert(face >= 0 && face < 6);
        return CardinalDirections.at(face);
    }

    constexpr ivec3 DirectionByCardinal(Direction direction)
    {
        return CardinalDirections.at(static_cast<int>(direction));
    }

    constexpr Direction VectorToCardinal(ivec3 direction)
    {
        if (direction == ivec3{1, 0, 0})
            return Direction::NORTH;
        if (direction == ivec3{0, 0, 1})
            return Direction::SOUTH;
        if (direction == ivec3{-1, 0, 0})
            return Direction::WEST;
        if (direction == ivec3{0, 0, -1})
            return Direction::EAST;
        if (direction == ivec3{0, 1, 0})
            return Direction::UP;
        if (direction == ivec3{0, -1, 0})
            return Direction::DOWN;
        assert(false && "Invalid direction vector");
        return Direction::NORTH;
    }
}
