#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>

#include <DirectXMath.h>

namespace Engine::Scene
{
	class Node
	{
	public:
		Node() : mLocalTransform(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f)
		{
		}

		void SetParent(const SharedPtr<Node>& parent) { mParent = parent; }

		void SetLocalTransform(const dx::XMFLOAT4X4& localTransform) { mLocalTransform = localTransform; }

		DirectX::XMMATRIX GetWorldTransform() const
		{
			DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&mLocalTransform);
			auto parent = mParent.lock();
			if (parent)
			{
				return DirectX::XMMatrixMultiply(parent->GetWorldTransform(), local);
			}
			return local;
		}

	private:
		WeakPtr<Node> mParent;
		DirectX::XMFLOAT4X4 mLocalTransform;
	};
}