#pragma once

#include <cstdint>

namespace Engine::Scene
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

        union
        {
            float FoV;
            struct
            {
                float OrthographicXMag;
                float OrthographicYMag;
            };
        };
    };
}