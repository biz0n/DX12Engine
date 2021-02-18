
#pragma once

#include <Types.h>
#include <MathUtils.h>

namespace Engine::Scene
{
    enum class CameraType : uint32
    {
        Perspective = 0,
        Orthographic = 1
    };

    class Camera
    {
    public: 
        Camera() : mType(CameraType::Perspective), mNearPlane(0.001f), mFarPlane(100.0f), mFoV(45 * Math::PI / 180.0f){}

        void SetNearPlane(float32 nearPlane) { mNearPlane = nearPlane; }
        float32 GetNearPlane() const { return mNearPlane; }

        void SetFarPlane(float32 farPlane) { mFarPlane = farPlane; }
        float32 GetFarPlane() const { return mFarPlane; }

        void SetFoV(float32 fov) { mFoV = fov; }
        float32 GetFoV() const { return mFoV; }

        void SetType(CameraType type) { mType = type; }
        CameraType GetType() const { return mType; }

        dx::XMMATRIX GetProjectionMatrix(float32 width, float32 height) const
        {
            if (mType == CameraType::Perspective)
            {
                return dx::XMMatrixPerspectiveFovLH(GetFoV(), width / height, GetNearPlane(), GetFarPlane());
            }
            else
            {
                return dx::XMMatrixOrthographicLH(width, height, GetNearPlane(), GetFarPlane());
            }
        }
    private:
        CameraType mType;
        float32 mNearPlane;
        float32 mFarPlane;
        float32 mFoV;
    };
}