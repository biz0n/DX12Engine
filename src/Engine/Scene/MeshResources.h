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

        SharedPtr<Memory::Buffer> meshletsBuffer;
        SharedPtr<Memory::Buffer> primitiveIndicesBuffer;
        SharedPtr<Memory::Buffer> uniqueVertexIndexBuffer;
        uint8 indexSize;
        uint32 indicesCount;
        uint32 meshletsCount;

        uint32 GetIndicesCount() const
        {
            return indicesCount;
        }

        uint32 GetMeshletsCount() const
        {
            return meshletsCount;
        }
    };
}