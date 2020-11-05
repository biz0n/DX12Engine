#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace Engine::Scene::Components
{
    struct AABBComponent
    {
        DirectX::BoundingBox boundingBox;
    };
}