#pragma once

#include <DirectXMath.h>

namespace Engine::Scene
{
    enum class LightType : uint8_t
    {
        DirectionalLight = 0,
        PointLight = 1,
        SpotLight = 2
    };

    struct PunctualLight
    {
        LightType LightType;
        DirectX::XMFLOAT3 Color;
        float Intensity;
        float ConstantAttenuation;
        float LinearAttenuation;
        float QuadraticAttenuation;
        float InnerConeAngle;
        float OuterConeAngle;
    };
}