#pragma once

#include <Scene/DataRegion.h>
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
        DataRegion NameIndex;
        
        DirectX::XMFLOAT4X4 LocalTransform;
        
        union
        {
            Engine::Scene::DataRegion MeshIndices;
            uint32_t LightIndex;
            uint32_t CameraIndex;
        };
    };
}