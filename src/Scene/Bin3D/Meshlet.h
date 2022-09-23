#pragma once

#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
#include <cstdint>

namespace Bin3D
{
    struct Meshlet
    {
        uint32_t VertCount;
        uint32_t VertOffset;
        uint32_t PrimCount;
        uint32_t PrimOffset;
    };

    struct MeshletTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
        uint32_t _unused : 2;
    };

    struct CullData
    {
        DirectX::BoundingSphere             BoundingSphere; // xyz = center, w = radius
        DirectX::PackedVector::XMUBYTEN4    NormalCone;     // xyz = axis, w = -cos(a + 90)
        float                               ApexOffset;     // apex = center - axis * offset
    };
}