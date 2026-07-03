#pragma once
#include "EventQueue.hpp"
#include "WorldEvents.hpp"

namespace lve
{
    struct EventBus
    {
        EventQueue<BlockBrokenEvent> blockBroken;
        EventQueue<BlockPlaceRequest> blockPlaceRequested;
    };
};
