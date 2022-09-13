#pragma once

#include <Scene/ArrayIndex.h>
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
        Engine::Scene::ArrayIndex MeshIndices;
        std::optional<uint32_t> LightIndex;
        std::optional<uint32_t> CameraIndex;
        DirectX::XMMATRIX LocalTransform;
        Engine::Scene::ArrayIndex NameIndex;
    };
}