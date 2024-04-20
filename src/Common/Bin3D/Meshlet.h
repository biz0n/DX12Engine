#pragma once

#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
#include <cstdint>

namespace Bin3D
{
    struct CullData
    {
        DirectX::BoundingSphere             BoundingSphere; // xyz = center, w = radius
        DirectX::PackedVector::XMUBYTEN4    NormalCone;     // xyz = axis, w = -cos(a + 90)
        float                               ApexOffset;     // apex = center - axis * offset
    };

    struct ClusterError
    {
        DirectX::XMFLOAT3 Center;
        float Error;
    };

    struct Meshlet
    {
        uint32_t VertCount;
        uint32_t VertOffset;
        uint32_t PrimCount;
        uint32_t PrimOffset;
        uint32_t GroupId = 0;
        uint32_t Lod = 0;

        ClusterError Error;
        ClusterError ParentClusterError;

        CullData CullData;
    };

    struct MeshletTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
        uint32_t _unused : 2;
    };

    
}