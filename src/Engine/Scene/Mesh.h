#pragma once

#include <Types.h>
#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <Scene/SceneForwards.h>

#include <d3d12.h>

namespace Engine::Scene
{
	class Mesh
	{
	public:
		SharedPtr<IndexBuffer> indexBuffer;
		SharedPtr<VertexBuffer> vertexBuffer;
		SharedPtr<Material> material;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
	};
}