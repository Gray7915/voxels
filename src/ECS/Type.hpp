#pragma once

#include <bitset>
#include <cstdint>

// ECS types
using Entity = uint32_t;
const Entity MAX_ENTITIES = 100000;

using ComponentType = uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;
