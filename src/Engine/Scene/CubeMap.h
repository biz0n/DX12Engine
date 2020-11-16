#pragma once

#include <Types.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Scene/SceneForwards.h>

#include <d3d12.h>

namespace Engine::Scene
{
    class CubeMap
    {
    public:
        SharedPtr<IndexBuffer> indexBuffer;
        SharedPtr<VertexBuffer> vertexBuffer;
        SharedPtr<Texture> texture;
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
    };
}