#pragma once

#include <Types.h>

namespace Engine::Scene::Components
{
    struct MeshComponent
    {
        Index MeshIndex;
        Size IndicesCount;
        Size VerticesCount;
        Index MaterialIndex;
    };
}