#pragma once

#include <DirectXMath.h>
#include <Scene/PunctualLight.h>

namespace Engine::Scene::Components
{
    struct LightComponent
    {
        PunctualLight light;
    };
}