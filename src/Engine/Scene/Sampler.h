#pragma once

#include <Types.h>
#include <Hash.h>
#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine::Scene
{
    struct Sampler
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

        Filter MagFilter = Filter::Linear;
        Filter MinFilter = Filter::Linear;
        Filter MipFilter = Filter::Linear;
        uint32 MaxAnisotropy = 1;
        float MaxLod = 1000;
        float MinLod = -1000;
        float LodBias = 0;
        ComparisonMode ComparisonFunc = ComparisonMode::Disabled;
        AddressMode ModeU = AddressMode::Wrap;
        AddressMode ModeV = AddressMode::Wrap;
        AddressMode ModeW = AddressMode::Wrap;
        dx::XMFLOAT4 BorderColor = {0, 0, 0, 0};

        D3D12_SAMPLER_DESC GetDesc() const
        {
            D3D12_SAMPLER_DESC desc{};

            desc.Filter = GetFilter(
                    MinFilter,
                    MagFilter,
                    MipFilter,
                    (ComparisonFunc != ComparisonMode::Disabled),
                    (MaxAnisotropy > 1));
            desc.AddressU = ConvertAddressMode(ModeU);
            desc.AddressV = ConvertAddressMode(ModeV);
            desc.AddressW = ConvertAddressMode(ModeW);

            desc.ComparisonFunc = ConvertComparisonFunc(ComparisonFunc);

            desc.MinLOD = MinLod;
            desc.MaxLOD = MaxLod;
            desc.MipLODBias = LodBias;

            desc.MaxAnisotropy = MaxAnisotropy;

            memcpy(desc.BorderColor, &BorderColor, sizeof(BorderColor));

            return desc;
        }

    private:
        D3D12_TEXTURE_ADDRESS_MODE ConvertAddressMode(AddressMode mode) const
        {
            switch (mode)
            {
                case AddressMode::Wrap:
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case AddressMode::Mirror:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case AddressMode::Clamp:
                    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case AddressMode::Border:
                    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                case AddressMode::MirrorOnce:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            }
        }

        D3D12_FILTER_TYPE ConvertFilterType(Filter filter) const
        {
            switch (filter)
            {
                case Filter::Point:
                    return D3D12_FILTER_TYPE_POINT;
                case Filter::Linear:
                    return D3D12_FILTER_TYPE_LINEAR;
                default:
                    return (D3D12_FILTER_TYPE)-1;
            }
        }

        D3D12_FILTER GetFilter(Filter minFilter, Filter magFilter, Filter mipFilter, bool isComparison, bool isAnisotropic) const
        {
            D3D12_FILTER filter;
            D3D12_FILTER_REDUCTION_TYPE reduction = isComparison ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD;

            if (isAnisotropic)
            {
                filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
            }
            else
            {
                D3D12_FILTER_TYPE dxMin = ConvertFilterType(minFilter);
                D3D12_FILTER_TYPE dxMag = ConvertFilterType(magFilter);
                D3D12_FILTER_TYPE dxMip = ConvertFilterType(mipFilter);
                filter = D3D12_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, reduction);
            }

            return filter;
        }

        D3D12_COMPARISON_FUNC ConvertComparisonFunc(ComparisonMode func) const
        {
            switch (func)
            {
                case ComparisonMode::Never:
                    return D3D12_COMPARISON_FUNC_NEVER;
                case ComparisonMode::Disabled:
                case ComparisonMode::Always:
                    return D3D12_COMPARISON_FUNC_ALWAYS;
                case ComparisonMode::Less:
                    return D3D12_COMPARISON_FUNC_LESS;
                case ComparisonMode::Equal:
                    return D3D12_COMPARISON_FUNC_EQUAL;
                case ComparisonMode::NotEqual:
                    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case ComparisonMode::LessEqual:
                    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case ComparisonMode::Greater:
                    return D3D12_COMPARISON_FUNC_GREATER;
                case ComparisonMode::GreaterEqual:
                    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                default:
                    return (D3D12_COMPARISON_FUNC)0;
            }
        }
    };
}

namespace std
{
    template <>
    struct hash<Engine::Scene::Sampler>
    {
        size_t operator()(const Engine::Scene::Sampler &key) const
        {
            return std::hash_combine(
                    key.MagFilter,
                    key.MinFilter,
                    key.MipFilter,
                    key.MaxAnisotropy,
                    key.MaxLod,
                    key.MinLod,
                    key.LodBias,
                    key.ComparisonFunc,
                    key.ModeU,
                    key.ModeV,
                    key.ModeW,
                    key.BorderColor.x,
                    key.BorderColor.y,
                    key.BorderColor.z,
                    key.BorderColor.w);
        }
    };
}
