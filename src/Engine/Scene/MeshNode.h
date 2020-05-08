#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>

#include <vector>

namespace Engine::Scene
{
	class MeshNode : public Node
	{
	public:
		MeshNode() : Node()
		{
		}

		const std::vector<SharedPtr<Mesh>>& GetMeshes() const { return mMeshes; }
		void SetMeshes(const std::vector<SharedPtr<Mesh>>& meshes) { mMeshes = std::move(meshes); }

	private:
		std::vector<SharedPtr<Mesh>> mMeshes;
	};
}