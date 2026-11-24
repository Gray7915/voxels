#pragma once
#include <chrono>
#include <vector>
#include "Util/Types.hpp"

struct InteractionTarget {
    vec3 targetPosition;
    BlockId targetId;
    double percentBroken;
    double progressSeconds;
    std::chrono::steady_clock::time_point startTime{};
    std::chrono::steady_clock::time_point lastInteractedwith{};
};

struct InteractionProgress {
    std::vector<InteractionTarget> InteractionTargetList;
};