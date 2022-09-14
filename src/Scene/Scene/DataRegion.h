#pragma once

#include <cstdint>

namespace Engine::Scene
{
    struct DataRegion
    {
        uint32_t Offset;
        uint32_t Size;
    };
}