#pragma once

namespace Math
{
    constexpr float PI = 3.1415926535897932384626433832795f;
    constexpr float _2PI = 2.0f * PI;

    template<typename T>
    T WrapAngle( T theta ) noexcept
    {
        constexpr T twoPi = (T)_2PI;
        const T mod = fmod( theta,twoPi );
        if( mod > (T)PI )
        {
            return mod - twoPi;
        }
        else if( mod < -(T)PI )
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
}