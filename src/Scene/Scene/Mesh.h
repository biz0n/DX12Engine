#pragma once

#include <Scene/DataRegion.h>
#include <DirectXCollision.h>

namespace Engine::Scene
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