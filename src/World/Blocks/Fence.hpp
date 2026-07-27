#pragma once
#include <iostream>

#include "Util/Types.hpp"
#include "Util/Direction.hpp"

#include "World/voxel.hpp"

namespace lve
{
    class Fence
    {
    public:
        /*
            In the case of something like a fence or wall the bits are assumed to be read as such
            Bit 0 = North
            Bit 1 = South
            Bit 2 = East
            Bit 3 = West
            Bit 4 = Up
            The central post of the fence or wall should always be drawn
        */
        static bool isSegmentBitActive(Voxel voxel, Math::Direction direction)
        {

            u8 segmentBits = voxel.state.segment;
            for (int i = 0; i < 5; i++)
            {
                bool set = (voxel.state.segment >> i) & 1;
                //std::cout << "bit " << i << " = " << set << '\n';
            }
            switch (direction)
            {
            case Math::Direction::NORTH:
                //std::cout << "north" << (voxel.state.segment & (1 << 0)) << '\n';
                return voxel.state.segment & (1 << 0);
                break;

            case Math::Direction::SOUTH:
                //std::cout << "south " << (voxel.state.segment & (1 << 1)) << '\n';
                return voxel.state.segment & (1 << 1);
                break;

            case Math::Direction::EAST:
                //std::cout << "east " << (voxel.state.segment & (1 << 2)) << '\n';
                return voxel.state.segment & (1 << 2);
                break;

            case Math::Direction::WEST:
                //std::cout << "west " << (voxel.state.segment & (1 << 3)) << '\n';
                return voxel.state.segment & (1 << 3);
                break;

            case Math::Direction::UP:
                //std::cout << "up " << (voxel.state.segment & (1 << 4)) << '\n';
                return voxel.state.segment & (1 << 4);
                break;
            case Math::Direction::DOWN:
                return true;
            }
            return false;
        }

        static void setSegmentBit(Voxel &voxel, Math::Direction direction, bool hasNeighbor)
        {
            switch (direction)
            {
            case Math::Direction::NORTH:
                hasNeighbor ? voxel.state.segment |= (1 << 0) : voxel.state.segment &= ~(1 << 0);
                break;

            case Math::Direction::SOUTH:
                hasNeighbor ? voxel.state.segment |= (1 << 1) : voxel.state.segment &= ~(1 << 1);
                break;

            case Math::Direction::EAST:
                hasNeighbor ? voxel.state.segment |= (1 << 2) : voxel.state.segment &= ~(1 << 2);
                break;

            case Math::Direction::WEST:
                hasNeighbor ? voxel.state.segment |= (1 << 3) : voxel.state.segment &= ~(1 << 3);
                break;

            case Math::Direction::UP:
                hasNeighbor ? voxel.state.segment |= (1 << 4) : voxel.state.segment &= ~(1 << 4);
                break;
            }
        }
    };
}
