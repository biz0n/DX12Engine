#pragma once

#include <Types.h>

namespace Engine
{
    struct Size2D
    {
        uint32 X;
        uint32 Y;

        Size2D(uint32 x, uint32 y) : X{x}, Y{y}
        {
        }
    };

    struct Size3D
    {
        uint32 X;
        uint32 Y;
        uint32 Z;

        Size3D(uint32 x, uint32 y, uint32 z) : X{x}, Y{y}, Z{z}
        {
        }

        Size3D(uint32 x, uint32 y) : X{x}, Y{y}, Z{0}
        {
        }
    };

}
