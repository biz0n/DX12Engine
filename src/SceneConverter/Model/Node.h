#pragma once

#include <Scene/DataRegion.h>
#include <Scene/Node.h>
#include <DirectXMath.h>

#include <vector>
#include <optional>

namespace SceneConverter::Model
{
    struct Node;
    struct Node
    {
        Engine::Scene::Node::NodeType Type;
        std::vector<Node> Children;
        Engine::Scene::DataRegion MeshIndices;
        std::optional<uint32_t> LightIndex;
        std::optional<uint32_t> CameraIndex;
        DirectX::XMFLOAT4X4 LocalTransform;
        Engine::Scene::DataRegion NameIndex;
    };
}