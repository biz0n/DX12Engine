#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>

#include <d3d12.h>

namespace Engine::Scene
{
    struct MeshResources
    {
        SharedPtr<Memory::IndexBuffer> indexBuffer;
        SharedPtr<Memory::VertexBuffer> vertexBuffer;
    };
}