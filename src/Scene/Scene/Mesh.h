#pragma once

#include <Scene/ArrayIndex.h>
#include <DirectXCollision.h>

namespace Engine::Scene
{
    struct Mesh
    {
        ArrayIndex NameIndex;
        uint32_t MaterialIndex;

        ArrayIndex Vertices;
        ArrayIndex Indices;
        
        DirectX::BoundingBox AABB;
    };
}