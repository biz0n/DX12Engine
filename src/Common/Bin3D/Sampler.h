#pragma once

#include <compare>

namespace Bin3D
{
    struct Sampler
    {
        enum class Filter : uint8_t
        {
            Point,
            Linear,
        };

        enum class AddressMode : uint8_t
        {
            Wrap,
            Mirror,
            Clamp,
            Border,
            MirrorOnce
        };

        enum class ComparisonMode : uint8_t
        {
            Disabled,
            Never,
            Always,
            Less,
            Equal,
            NotEqual,
            LessEqual,
            Greater,
            GreaterEqual,
        };

        Filter MagFilter = Filter::Linear;
        Filter MinFilter = Filter::Linear;
        Filter MipFilter = Filter::Linear;
        uint8_t MaxAnisotropy = 1;
        float MaxLod = 1000;
        float MinLod = -1000;
        float LodBias = 0;
        ComparisonMode ComparisonFunc = ComparisonMode::Disabled;
        AddressMode ModeU = AddressMode::Wrap;
        AddressMode ModeV = AddressMode::Wrap;
        AddressMode ModeW = AddressMode::Wrap;
        float BorderColor[4] = {0, 0, 0, 0};

        auto operator<=>(const Sampler& other) const = default;
    };
}
