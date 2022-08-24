#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>

#include <d3d12.h>

namespace Engine::Scene
{
    struct Mesh
    {
        SharedPtr<Memory::IndexBuffer> indexBuffer;
        SharedPtr<Memory::VertexBuffer> vertexBuffer;
        Size indicesCount;
        Size verticesCount;
    };
}