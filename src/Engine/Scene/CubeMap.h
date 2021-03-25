#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>

#include <d3d12.h>

namespace Engine::Scene
{
    class CubeMap
    {
    public:
        SharedPtr<Memory::IndexBuffer> indexBuffer;
        SharedPtr<Memory::VertexBuffer> vertexBuffer;
        SharedPtr<Memory::Texture> texture;
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
    };
}