#pragma once

#include <DirectXMath.h>

namespace Engine::Scene::Components
{
    struct LocalTransformComponent
    {
        DirectX::XMMATRIX transform;
    };
}