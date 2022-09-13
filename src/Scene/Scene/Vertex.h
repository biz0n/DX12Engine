#pragma once

#include <DirectXMath.h>

namespace Engine::Scene
{
    struct Vertex
    {
        DirectX::XMFLOAT3 Vertex;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TextureCoord;
        DirectX::XMFLOAT4 Tangent;
    };
} // namespace Engine::Scene