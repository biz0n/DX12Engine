#pragma once

#include <DirectXMath.h>

namespace Engine
{
    namespace Math
    {
        constexpr float PI = 3.1415926535897932384626433832795f;
        constexpr float _2PI = 2.0f * PI;

        template <typename T>
        T WrapAngle(T theta) noexcept
        {
            constexpr T twoPi = (T)_2PI;
            const T mod = fmod(theta, twoPi);
            if (mod > (T)PI)
            {
                return mod - twoPi;
            }
            else if (mod < -(T)PI)
            {
                return mod + twoPi;
            }
            return mod;
        }

        template <typename T>
        inline T AlignUpWithMask(T value, size_t mask)
        {
            return (T)(((size_t)value + mask) & ~mask);
        }

        template <typename T>
        inline T AlignDownWithMask(T value, size_t mask)
        {
            return (T)((size_t)value & ~mask);
        }

        template <typename T>
        inline T AlignUp(T value, size_t alignment)
        {
            return AlignUpWithMask(value, alignment - 1);
        }

        template <typename T>
        inline T AlignDown(T value, size_t alignment)
        {
            return AlignDownWithMask(value, alignment - 1);
        }

        inline void ExtractPitchYawRollFromXMMatrix(float* flt_p_PitchOut, float* flt_p_YawOut, float* flt_p_RollOut, const DirectX::XMMATRIX* XMMatrix_p_Rotation)
        {
            DirectX::XMFLOAT4X4 XMFLOAT4X4_Values;
            DirectX::XMStoreFloat4x4(&XMFLOAT4X4_Values, DirectX::XMMatrixTranspose(*XMMatrix_p_Rotation));
            *flt_p_PitchOut = (float)asin(-XMFLOAT4X4_Values._23);
            *flt_p_YawOut = (float)atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33);
            *flt_p_RollOut = (float)atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22);
        }
    } // namespace Math

} // namespace Engine