#pragma once
#include "ECS/Coordinator.hpp"
namespace lve
{
    class Area;
    class ChunkMutationSystem {
      public:
        void Update(Area &area, Coordinator &coordinator);
    };
} // namespace lve
