#pragma once

#include "Util/Types.hpp"
class RenderStats
{
public:
    static RenderStats &Get()
    {
        static RenderStats instance;
        return instance;
    }
    u32 drawCalls = 0;
    u64 triangles = 0;

    void reset()
    {
        drawCalls = 0;
        triangles = 0;
    }
};
