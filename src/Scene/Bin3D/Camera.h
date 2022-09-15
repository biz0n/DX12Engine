#pragma once

#include <cstdint>

namespace Bin3D
{
    enum class CameraType : uint8_t
    {
        Perspective = 0,
        Orthographic = 1
    };

    struct Camera
    {
        CameraType Type;
        float NearPlane;
        float FarPlane;

        float FoV;
        float OrthographicXMag;
        float OrthographicYMag;
    };
}