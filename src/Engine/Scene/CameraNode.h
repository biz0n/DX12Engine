#pragma once

#include <Scene/SceneForwards.h>

#include <MathUtils.h>

namespace Engine::Scene
{
    class CameraNode : public Node
    {
    public:
        CameraNode() : Node(), mNearPlane(0.001f), mFarPlane(100.0f), mFoV(45 * Math::PI / 180.0f), mAspectRatio(1), mTranslation{0}, mPitch(0), mYaw(0)
        {
        }

        void SetNearPlane(float32 nearPlane) { mNearPlane = nearPlane; }
        float32 GetNearPlane() const { return mNearPlane; }

        void SetFarPlane(float32 farPlane) { mFarPlane = farPlane; }
        float32 GetFarPlane() const { return mFarPlane; }

        void SetFoV(float32 fov) { mFoV = fov; }
        float32 GetFoV() const { return mFoV; }

        void SetAspectRatio(float32 aspectRatio) { mAspectRatio = aspectRatio; }

        dx::XMMATRIX GetViewMatrix() const
        {
            using namespace dx;

            const dx::XMVECTOR up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            const dx::XMVECTOR forward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

            dx::XMVECTOR rotation;
            dx::XMVECTOR translation;
            CalculateTranslateAndRotation(&translation, &rotation);

            auto lookAtDirection = dx::XMVector3Transform(
                forward,
                dx::XMMatrixRotationQuaternion(rotation));

            auto eyePosition = translation + mTranslation;

            return dx::XMMatrixLookAtLH(eyePosition, eyePosition + lookAtDirection, up);
        }

        dx::XMMATRIX GetProjectionMatrix() const
        {
            return dx::XMMatrixPerspectiveFovLH(mFoV, mAspectRatio, mNearPlane, mFarPlane);
        }

        dx::XMFLOAT3 GetPosition() const
        {
            auto matrix = GetWorldTransform();

            dx::XMVECTOR s;
            dx::XMVECTOR r;
            dx::XMVECTOR t;
            dx::XMMatrixDecompose(&s, &r, &t, matrix);

            dx::XMFLOAT3 position;
            dx::XMStoreFloat3(&position, t);
            return position;
        }

        void Rotate(float32 dx, float32 dy)
        {
            mYaw = Math::WrapAngle(mYaw + dx);

            mPitch = std::clamp(mPitch + dy, -0.995f * Math::PI / 2.0f, +0.995f * Math::PI / 2.0f);
        }

        void Translate(DirectX::XMFLOAT3 translate)
        {
            using namespace dx;

            dx::XMVECTOR rotation;
            dx::XMVECTOR translation;
            CalculateTranslateAndRotation(&translation, &rotation);

            auto positionOffset = DirectX::XMVector3Transform(
                DirectX::XMLoadFloat3(&translate),
                dx::XMMatrixRotationQuaternion(rotation));

            mTranslation += positionOffset;
        }

    private:
        void CalculateTranslateAndRotation(dx::XMVECTOR *translation, dx::XMVECTOR *rotation) const
        {
            using namespace dx;
            auto matrix = GetWorldTransform();

            dx::XMVECTOR s;
            dx::XMVECTOR r;
            dx::XMVECTOR t;
            dx::XMMatrixDecompose(&s, &r, translation, matrix);

            auto pitchYawRotation = dx::XMQuaternionRotationRollPitchYaw(mPitch, mYaw, 0.0f);
            *rotation = dx::XMQuaternionMultiply(pitchYawRotation, r);
        }

    private:
        float32 mNearPlane;
        float32 mFarPlane;
        float32 mFoV;
        float32 mAspectRatio;

        dx::XMVECTOR mTranslation;
        float32 mPitch;
        float32 mYaw;
    };
} // namespace Engine::Scene