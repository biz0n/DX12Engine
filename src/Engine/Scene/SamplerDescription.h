#pragma once

#include <Types.h>
#include <Bin3D/Sampler.h>
#include <d3d12.h>

namespace Engine::Scene
{
    struct SamplerDescription
    {
        D3D12_SAMPLER_DESC GetDesc(const Bin3D::Sampler& sampler) const
        {
            D3D12_SAMPLER_DESC desc{};

            desc.Filter = GetFilter(
                sampler.MinFilter,
                sampler.MagFilter,
                sampler.MipFilter,
                    (sampler.ComparisonFunc != Bin3D::Sampler::ComparisonMode::Disabled),
                    (sampler.MaxAnisotropy > 1));
            desc.AddressU = ConvertAddressMode(sampler.ModeU);
            desc.AddressV = ConvertAddressMode(sampler.ModeV);
            desc.AddressW = ConvertAddressMode(sampler.ModeW);

            desc.ComparisonFunc = ConvertComparisonFunc(sampler.ComparisonFunc);

            desc.MinLOD = sampler.MinLod;
            desc.MaxLOD = sampler.MaxLod;
            desc.MipLODBias = sampler.LodBias;

            desc.MaxAnisotropy = sampler.MaxAnisotropy;

            memcpy(desc.BorderColor, &sampler.BorderColor, sizeof(float) * 4);

            return desc;
        }

    private:
        D3D12_TEXTURE_ADDRESS_MODE ConvertAddressMode(Bin3D::Sampler::AddressMode mode) const
        {
            switch (mode)
            {
                case Bin3D::Sampler::AddressMode::Wrap:
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case Bin3D::Sampler::AddressMode::Mirror:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case Bin3D::Sampler::AddressMode::Clamp:
                    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case Bin3D::Sampler::AddressMode::Border:
                    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                case Bin3D::Sampler::AddressMode::MirrorOnce:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            }
        }

        D3D12_FILTER_TYPE ConvertFilterType(Bin3D::Sampler::Filter filter) const
        {
            switch (filter)
            {
                case Bin3D::Sampler::Filter::Point:
                    return D3D12_FILTER_TYPE_POINT;
                case Bin3D::Sampler::Filter::Linear:
                    return D3D12_FILTER_TYPE_LINEAR;
                default:
                    return (D3D12_FILTER_TYPE)-1;
            }
        }

        D3D12_FILTER GetFilter(
            Bin3D::Sampler::Filter minFilter, 
            Bin3D::Sampler::Filter magFilter, 
            Bin3D::Sampler::Filter mipFilter, 
            bool isComparison, 
            bool isAnisotropic) const
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

        D3D12_COMPARISON_FUNC ConvertComparisonFunc(Bin3D::Sampler::ComparisonMode func) const
        {
            switch (func)
            {
                case Bin3D::Sampler::ComparisonMode::Never:
                    return D3D12_COMPARISON_FUNC_NEVER;
                case Bin3D::Sampler::ComparisonMode::Disabled:
                case Bin3D::Sampler::ComparisonMode::Always:
                    return D3D12_COMPARISON_FUNC_ALWAYS;
                case Bin3D::Sampler::ComparisonMode::Less:
                    return D3D12_COMPARISON_FUNC_LESS;
                case Bin3D::Sampler::ComparisonMode::Equal:
                    return D3D12_COMPARISON_FUNC_EQUAL;
                case Bin3D::Sampler::ComparisonMode::NotEqual:
                    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case Bin3D::Sampler::ComparisonMode::LessEqual:
                    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case Bin3D::Sampler::ComparisonMode::Greater:
                    return D3D12_COMPARISON_FUNC_GREATER;
                case Bin3D::Sampler::ComparisonMode::GreaterEqual:
                    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                default:
                    return (D3D12_COMPARISON_FUNC)0;
            }
        }
    };
}