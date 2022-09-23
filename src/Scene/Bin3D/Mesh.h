#pragma once

#include <Bin3D/DataRegion.h>
#include <DirectXCollision.h>

namespace Bin3D
{
    struct Mesh
    {
        uint32_t MaterialIndex;

        DataRegion Vertices;
        DataRegion Indices;

        DataRegion Meshlets;
        DataRegion PrimitiveIndices;
        DataRegion UniqueVertexIndices;

        uint8_t IndexSize = 4; //bytes
        
        DirectX::BoundingBox AABB;
    };
}