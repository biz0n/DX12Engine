#pragma once

#include <Bin3D/DataRegion.h>
#include <DirectXMath.h>

namespace Bin3D
{
    struct Node
    {
        enum class NodeType : uint8_t
        {
            Node,
            Mesh,
            Light,
            Camera
        };

        NodeType Type;

        uint32_t Parent;
        DataRegion NameIndex;
        
        DirectX::XMFLOAT4X4 LocalTransform;
        
        Bin3D::DataRegion DataIndex;
    };
}