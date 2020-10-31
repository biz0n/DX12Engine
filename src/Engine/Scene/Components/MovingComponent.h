#pragma once

#include <Types.h>
#include <DirectXMath.h>

#include <MathUtils.h>
#include <algorithm>

namespace Engine::Scene::Components
{
    struct MovingComponent
    {
        void Rotate(float32 dx, float32 dy)
        {
            mYaw = Math::WrapAngle(mYaw + dx);
            mPitch = std::clamp(mPitch + dy, -0.995f * Math::PI / 2.0f, +0.995f * Math::PI / 2.0f);
        }

        float32 mPitch{0};
        float32 mYaw{0};
    };
}