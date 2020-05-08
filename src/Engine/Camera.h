#pragma once

#include <Types.h>
#include <MathUtils.h>

#include <DirectXMath.h>

namespace Engine
{
    class Camera
    {
    public:
        Camera(DirectX::XMFLOAT3 position, float32 pitch, float32 yaw);

        DirectX::XMMATRIX GetMatrix() const;

        DirectX::XMFLOAT3 GetPosition() const;

        DirectX::XMFLOAT3 GetTarget() const;

        void Reset();

        void Rotate(float32 dx, float32 dy);

        void Translate(DirectX::XMFLOAT3 translate);

    private:
        DirectX::XMFLOAT3 mPosition;
        float32 mPitch;
        float32 mYaw;

        DirectX::XMFLOAT3 mHomePosition;
        float32 mHomePitch;
        float32 mHomeYaw;
    };

} // namespace Engine