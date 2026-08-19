#pragma once
#include <iostream>

#include "Util/Types.hpp"
#include "Util/Direction.hpp"

#include "World/voxel.hpp"

class StagedBlock {
  public:
    static u8 getBlockStage(const Voxel &voxel) { return voxel.state.usagebits; }

    static void setBlockStage(Voxel &voxel, u8 stage) { voxel.state.usagebits = stage; }
};