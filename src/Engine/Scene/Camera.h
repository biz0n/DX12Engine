
#pragma once

#include <Types.h>
#include <MathUtils.h>

namespace Engine::Scene
{
    class Camera
    {
    public: 
        Camera() : mNearPlane(0.001f), mFarPlane(100.0f), mFoV(45 * Math::PI / 180.0f){}

        void SetNearPlane(float32 nearPlane) { mNearPlane = nearPlane; }
        float32 GetNearPlane() const { return mNearPlane; }

        void SetFarPlane(float32 farPlane) { mFarPlane = farPlane; }
        float32 GetFarPlane() const { return mFarPlane; }

        void SetFoV(float32 fov) { mFoV = fov; }
        float32 GetFoV() const { return mFoV; }

        dx::XMMATRIX GetProjectionMatrix(float32 aspectRation) const
        {
            return dx::XMMatrixPerspectiveFovLH(GetFoV(), aspectRation, GetNearPlane(), GetFarPlane());
        }

    private:
        float32 mNearPlane;
        float32 mFarPlane;
        float32 mFoV;
    };
}