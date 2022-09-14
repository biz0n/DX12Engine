#pragma once

#include <Bin3D/DataRegion.h>
#include <DirectXCollision.h>

namespace Bin3D
{
    struct Mesh
    {
        DataRegion NameIndex;
        uint32_t MaterialIndex;

        DataRegion Vertices;
        DataRegion Indices;
        
        DirectX::BoundingBox AABB;
    };
}