#pragma once

#include <DirectXMath.h>
#include <d3dx12.h>

#include <vector>

namespace Engine::Scene
{
    struct Vertex
    {
        DirectX::XMFLOAT3 Vertex;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TextureCoord;
        DirectX::XMFLOAT4 Tangent;

        static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout()
        {
            return 
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
            };
        }
    };
} // namespace Engine::Scene