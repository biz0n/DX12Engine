#pragma once

#include <DirectXMath.h>

namespace Engine::Scene::Components
{
    struct WorldTransformComponent
    {
        DirectX::XMMATRIX transform;
    };
}