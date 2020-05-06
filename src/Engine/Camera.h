#pragma once

#include <Types.h>
#include <MathUtils.h>

#include <DirectXMath.h>
#include <algorithm>

namespace Engine
{
    class Camera
    {
    public:
        Camera(DirectX::XMFLOAT3 position, float32 pitch, float32 yaw)
            : mHomePosition(position), mHomePitch(pitch), mHomeYaw(yaw)
        {
            Reset();
        }

        DirectX::XMMATRIX GetMatrix() const
        {
            using namespace DirectX;

            const DirectX::XMVECTOR forwardDirection = {0.0f, 0.0f, 1.0f, 0.0f};
            const auto lookVector = DirectX::XMVector4Transform(
                forwardDirection,
                DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

            const auto cameraPosition = DirectX::XMLoadFloat3(&mPosition);

            const auto target = lookVector + cameraPosition;

            return DirectX::XMMatrixLookAtLH(cameraPosition, target, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        }

        DirectX::XMFLOAT3 GetPosition() const
        {
            return mPosition;
        }

        DirectX::XMFLOAT3 GetTarget() const
        {
            using namespace DirectX;

            const DirectX::XMVECTOR forwardDirection = {0.0f, 0.0f, 1.0f, 0.0f};
            const auto lookVector = DirectX::XMVector4Transform(
                forwardDirection,
                DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

            const auto cameraPosition = DirectX::XMLoadFloat3(&mPosition);

            const auto target = lookVector + cameraPosition;

            DirectX::XMFLOAT3 t;
            DirectX::XMStoreFloat3(&t, lookVector);
            return t;
        }

        void Reset()
        {
            mPosition = mHomePosition;
            mPitch = mHomePitch;
            mYaw = mHomeYaw;
        }

        void Rotate(float32 dx, float32 dy)
        {
            mYaw = Math::WrapAngle(mYaw + dx);

            mPitch = std::clamp(mPitch + dy, -0.995f * Math::PI / 2.0f, +0.995f * Math::PI / 2.0f);
        }

        void Translate(DirectX::XMFLOAT3 translate)
        {
            using namespace DirectX;
            auto newPosition = DirectX::XMVector3Transform(
                DirectX::XMLoadFloat3(&translate),
                DirectX::XMMatrixRotationRollPitchYaw(mPitch, mYaw, 0.0f));

            newPosition += DirectX::XMLoadFloat3(&mPosition);
            DirectX::XMStoreFloat3(&mPosition, newPosition);
        }

    private:
        DirectX::XMFLOAT3 mPosition;
        float32 mPitch;
        float32 mYaw;

        DirectX::XMFLOAT3 mHomePosition;
        float32 mHomePitch;
        float32 mHomeYaw;
    };

} // namespace Engine