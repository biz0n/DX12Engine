#pragma once

#include <Types.h>
#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <Scene/SceneForwards.h>

namespace Engine::Scene
{
	class Mesh
	{
	public:
		IndexBuffer mIndexBuffer;
		VertexBuffer mVertexBuffer;
		SharedPtr<Material> material;
	};

	
}