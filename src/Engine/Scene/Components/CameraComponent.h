#pragma once

#include <DirectXMath.h>

namespace Engine::Scene::Components
{
    struct CameraComponent
    {
        float32 NearPlane;
        float32 FarPlane;
        float32 FoV;
        float32 AspectRatio;

        DirectX::XMVECTOR Translation;
        float32 Pitch;
        float32 Yaw;
    };
}