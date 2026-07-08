#pragma once
#include "EventQueue.hpp"
#include "WorldEvents.hpp"

namespace lve
{
    struct EventBus
    {
        EventQueue<BlockBrokenEvent> blockBreakRequest;
        EventQueue<BlockPlaceRequest> blockPlaceRequested;

        
        EventQueue<BlockHarvested> blockHarvestEvent;
    };
};
