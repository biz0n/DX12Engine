#pragma once

#include <Scene/ArrayIndex.h>
#include <DirectXMath.h>

namespace Engine::Scene
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
        ArrayIndex NameIndex;
        
        DirectX::XMMATRIX LocalTransform;
        
        union
        {
            Engine::Scene::ArrayIndex MeshIndices;
            uint32_t LightIndex;
            uint32_t CameraIndex;
        };
    };
}