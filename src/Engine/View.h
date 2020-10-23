#pragma once

#include <Types.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Engine
{
    struct View
    {
        HWND WindowHandle;
        uint32 width;
        uint32 height;
    };
}