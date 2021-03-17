#pragma once

#include <Types.h>

#include <DirectXMath.h>

namespace Engine::Scene
{
    enum class Filter
    {
        Point,
        Linear,
    };

    enum class AddressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    enum class ComparisonMode
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

    struct Sampler
    {
        Filter MagFilter = Filter::Linear;
        Filter MinFilter = Filter::Linear;
        Filter MipFilter = Filter::Linear;
        uint32 MaxAnisotropy = 1;
        float MaxLod = 1000;
        float MinLod = -1000;
        float LodBias = 0;
        ComparisonMode ComparisonMode = ComparisonMode::Disabled;
        AddressMode ModeU = AddressMode::Wrap;
        AddressMode ModeV = AddressMode::Wrap;
        AddressMode ModeW = AddressMode::Wrap;
        dx::XMFLOAT4 BorderColor = {0, 0, 0, 0};
    };

}
