#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>
#include <Memory/Buffer.h>

#include <d3d12.h>

namespace Engine::Scene
{
    struct MeshResources
    {
        SharedPtr<Memory::Buffer> indexBuffer;
        SharedPtr<Memory::Buffer> vertexCoordinatesBuffer;
        SharedPtr<Memory::Buffer> vertexPropertiesBuffer;

        uint32 GetIndicesCount() const
        {
            return indexBuffer->GetDescription().Width / indexBuffer->GetStride();
        }
    };
}