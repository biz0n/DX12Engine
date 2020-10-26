#pragma once

#include <DirectXMath.h>
#include <Scene/LightNode.h>

namespace Engine::Scene::Components
{
    struct LightComponent
    {
        Scene::LightType LightType;
		bool Enabled;
		DirectX::XMFLOAT3 Direction;
		DirectX::XMFLOAT3 Color;
		float ConstantAttenuation;
		float LinearAttenuation;
		float QuadraticAttenuation;
		float InnerConeAngle;
		float OuterConeAngle;
    };
}