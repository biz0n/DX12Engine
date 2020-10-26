#pragma once

#include <Types.h>
#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <Scene/Material.h>

#include <d3d12.h>

namespace Engine::Scene::Components
{
	struct MeshComponent
	{
		IndexBuffer IndexBuffer;
		VertexBuffer VertexBuffer;
		SharedPtr<Material> Material;
		D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;
	};
}