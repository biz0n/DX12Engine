#pragma once

#include <DirectXMath.h>

namespace Bin3D
{
    struct VertexCoordinates
    {
        DirectX::XMFLOAT3 Position;
    };

    struct VertexProperties
    {
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TextureCoord;
        DirectX::XMFLOAT4 Tangent;
    };
} // namespace Engine::Scene