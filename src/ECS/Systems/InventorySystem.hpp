#pragma once
#include "ECS/System.hpp"
#include "ECS/Components/InventoryComponent.hpp"
#include "World/Area.hpp"

namespace lve
{
    class InventorySystem : public System
    {
    public:
        void Update(Area &area);
    };
}
