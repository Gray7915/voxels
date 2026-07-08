#pragma once

namespace lve
{
    enum class ChunkState
    {
        Unloaded,
        QueuedForGeneration,
        Generating,
        Generated,
        QueuedForMeshing,
        Meshing,
        Ready,
        Uploaded,
        Dirty
    };
}
