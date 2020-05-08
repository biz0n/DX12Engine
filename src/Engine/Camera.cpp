#include "Camera.h"

#include <algorithm>

namespace Engine
{
	using namespace DirectX;

	Camera::Camera(DirectX::XMFLOAT3 position, float32 pitch, float32 yaw) : mHomePosition(position), mHomePitch(pitch), mHomeYaw(yaw)
	{
		Reset();
	}

	XMMATRIX Engine::Camera::GetMatrix() const
	{
		const DirectX::XMVECTOR forwardDirection = { 0.0f, 0.0f, 1.0f, 0.0f };
		const auto lookVector = DirectX::XMVector4Transform(
			forwardDirection,
			DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

		const auto cameraPosition = DirectX::XMLoadFloat3(&mPosition);

		const auto target = lookVector + cameraPosition;

		return DirectX::XMMatrixLookAtLH(cameraPosition, target, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	XMFLOAT3 Engine::Camera::GetPosition() const
	{
		return mPosition;
	}

	XMFLOAT3 Engine::Camera::GetTarget() const
	{
		const DirectX::XMVECTOR forwardDirection = { 0.0f, 0.0f, 1.0f, 0.0f };
		const auto lookVector = DirectX::XMVector4Transform(
			forwardDirection,
			DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

		const auto cameraPosition = DirectX::XMLoadFloat3(&mPosition);

		const auto target = lookVector + cameraPosition;

		DirectX::XMFLOAT3 t;
		DirectX::XMStoreFloat3(&t, lookVector);
		return t;
	}

	void Camera::Reset()
	{
		mPosition = mHomePosition;
		mPitch = mHomePitch;
		mYaw = mHomeYaw;
	}

	void Camera::Rotate(float32 dx, float32 dy)
	{
		mYaw = Math::WrapAngle(mYaw + dx);

		mPitch = std::clamp(mPitch + dy, -0.995f * Math::PI / 2.0f, +0.995f * Math::PI / 2.0f);
	}

	void Camera::Translate(DirectX::XMFLOAT3 translate)
	{
		auto newPosition = DirectX::XMVector3Transform(
			DirectX::XMLoadFloat3(&translate),
			DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

		newPosition += DirectX::XMLoadFloat3(&mPosition);
		DirectX::XMStoreFloat3(&mPosition, newPosition);
	}
}