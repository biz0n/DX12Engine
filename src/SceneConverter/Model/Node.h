#pragma once

#include <Bin3D/DataRegion.h>
#include <Bin3D/Node.h>
#include <DirectXMath.h>

#include <vector>
#include <optional>

namespace SceneConverter::Model
{
    struct Node;
    struct Node
    {
        Bin3D::Node::NodeType Type;
        std::vector<Node> Children;
        Bin3D::DataRegion MeshIndices;
        std::optional<uint32_t> LightIndex;
        std::optional<uint32_t> CameraIndex;
        DirectX::XMFLOAT4X4 LocalTransform;
        Bin3D::DataRegion NameIndex;
    };
}