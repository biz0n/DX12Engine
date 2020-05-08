#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>

namespace Engine::Scene
{
	class SceneObject
	{
	public:
		std::vector<SharedPtr<MeshNode>> nodes;

		std::vector<SharedPtr<LightNode>> lights;
	};
}