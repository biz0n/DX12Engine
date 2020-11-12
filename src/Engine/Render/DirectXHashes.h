#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <Hash.h>
#include <tuple>

inline bool operator<(const D3D_SHADER_MACRO &left, const D3D_SHADER_MACRO &right)
{
    auto tl = std::tie(left.Name, left.Definition);
    auto tr = std::tie(right.Name, right.Definition);

    return tl < tr;
}

inline bool operator<(const D3D12_INPUT_ELEMENT_DESC &left, const D3D12_INPUT_ELEMENT_DESC &right)
{
    auto tl = std::tie(
        left.AlignedByteOffset,
        left.Format,
        left.InputSlot,
        left.InputSlotClass,
        left.InstanceDataStepRate,
        left.SemanticIndex,
        left.SemanticName);

    auto tr = std::tie(
        right.AlignedByteOffset,
        right.Format,
        right.InputSlot,
        right.InputSlotClass,
        right.InstanceDataStepRate,
        right.SemanticIndex,
        right.SemanticName);

    return tl < tr;
}

namespace std
{
    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE &key) const
        {
            ID3D12RootSignature *desc = key;
            return std::hash<void *>{}(desc);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT &key) const
        {
            D3D12_INPUT_LAYOUT_DESC desc = key;
            return std::hash_combine(
                desc.NumElements,
                 desc.pInputElementDescs);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY &key) const
        {
            D3D12_PRIMITIVE_TOPOLOGY_TYPE desc = key;
            return desc;
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT &key) const
        {
            DXGI_FORMAT desc = key;
            return desc;
        }
    };

    template <>
    struct hash<D3D12_RASTERIZER_DESC>
    {
        size_t operator()(const D3D12_RASTERIZER_DESC &desc) const
        {
            return std::hash_combine(
                desc.AntialiasedLineEnable,
                desc.ConservativeRaster,
                desc.CullMode,
                desc.DepthBias,
                desc.DepthBiasClamp,
                desc.DepthClipEnable,
                desc.FillMode,
                desc.ForcedSampleCount,
                desc.FrontCounterClockwise,
                desc.MultisampleEnable,
                desc.SlopeScaledDepthBias);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER &key) const
        {
            D3D12_RASTERIZER_DESC desc = key;
            return std::hash<D3D12_RASTERIZER_DESC>{}(desc);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS &key) const
        {
            D3D12_RT_FORMAT_ARRAY desc = key;

            return std::hash_combine(
                desc.NumRenderTargets,
                std::hash_combine(desc.RTFormats));
        }
    };

    template <>
    struct hash<D3D12_SHADER_BYTECODE>
    {
        size_t operator()(const D3D12_SHADER_BYTECODE &key) const
        {
            return std::hash_combine(
                key.BytecodeLength,
                key.pShaderBytecode);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_VS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_VS &key) const
        {
            D3D12_SHADER_BYTECODE desc = key;

            return std::hash<D3D12_SHADER_BYTECODE>{}(desc);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_PS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_PS &key) const
        {
            D3D12_SHADER_BYTECODE desc = key;

            return std::hash<D3D12_SHADER_BYTECODE>{}(desc);
        }
    };

    template <>
    struct hash<D3D12_DEPTH_STENCILOP_DESC>
    {
        size_t operator()(const D3D12_DEPTH_STENCILOP_DESC &desc) const
        {
            return std::hash_combine(
                desc.StencilDepthFailOp,
                desc.StencilFailOp,
                desc.StencilFunc,
                desc.StencilPassOp);
        }
    };

    template <>
    struct hash<D3D12_DEPTH_STENCIL_DESC>
    {
        size_t operator()(const D3D12_DEPTH_STENCIL_DESC &desc) const
        {
            return std::hash_combine(
                desc.BackFace,
                desc.DepthEnable,
                desc.DepthFunc,
                desc.DepthWriteMask,
                desc.FrontFace,
                desc.StencilEnable,
                desc.StencilReadMask,
                desc.StencilWriteMask);
        }
    };

    template <>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL &key) const
        {
            D3D12_DEPTH_STENCIL_DESC desc = key;

            return std::hash<D3D12_DEPTH_STENCIL_DESC>{}(desc);
        }
    };

    template <>
    struct hash<D3D12_INPUT_ELEMENT_DESC>
    {
        size_t operator()(const D3D12_INPUT_ELEMENT_DESC &key) const
        {
            return std::hash_combine(
                key.AlignedByteOffset,
                key.Format,
                key.InputSlot,
                key.InputSlotClass,
                key.InstanceDataStepRate,
                key.SemanticIndex,
                key.SemanticName);
        }
    };

    template <>
    struct hash<D3D_SHADER_MACRO>
    {
        size_t operator()(const D3D_SHADER_MACRO &key) const
        {
            std::string name = key.Name;
            std::string definition = key.Definition;
            return std::hash_combine(
                std::hash<std::string>{}(name),
                std::hash<std::string>{}(definition));
        }
    };

    

    template <>
    struct hash<DXGI_SAMPLE_DESC>
    {
        size_t operator()(const DXGI_SAMPLE_DESC &key) const
        {
            return std::hash_combine(
                key.Count,
                key.Quality);
        }
    };

    template <>
    struct hash<D3D12_RESOURCE_DESC>
    {
        size_t operator()(const D3D12_RESOURCE_DESC &key) const
        {
            return std::hash_combine(
                key.Alignment,
                key.DepthOrArraySize,
                key.Dimension,
                key.Flags,
                key.Format,
                key.Height,
                key.Layout,
                key.MipLevels,
                key.SampleDesc,
                key.Width);
        }
    };

    template <>
    struct hash<D3D12_DEPTH_STENCIL_VALUE>
    {
        size_t operator()(const D3D12_DEPTH_STENCIL_VALUE &key) const
        {
            return std::hash_combine(
                key.Depth,
                key.Stencil);
        }
    };

    template <>
    struct hash<D3D12_CLEAR_VALUE>
    {
        size_t operator()(const D3D12_CLEAR_VALUE &key) const
        {
            return std::hash_combine(
                std::hash_combine(key.Color),
                key.DepthStencil,
                key.Format);
        }
    };

    template<>
    struct hash<D3D12_SHADER_RESOURCE_VIEW_DESC>
    {
        std::size_t operator()(const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc) const noexcept
        {
            std::size_t seed = 0;

            seed = hash_combine_impl(seed, 
                srvDesc.Format,
                srvDesc.ViewDimension,
                srvDesc.Shader4ComponentMapping);

            switch (srvDesc.ViewDimension)
            {
            case D3D12_SRV_DIMENSION_BUFFER:
                seed = hash_combine_impl(seed, 
                    srvDesc.Buffer.FirstElement,
                    srvDesc.Buffer.NumElements,
                    srvDesc.Buffer.StructureByteStride,
                    srvDesc.Buffer.Flags);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE1D:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture1D.MostDetailedMip,
                    srvDesc.Texture1D.MipLevels,
                    srvDesc.Texture1D.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture1DArray.MostDetailedMip,
                    srvDesc.Texture1DArray.MipLevels,
                    srvDesc.Texture1DArray.FirstArraySlice,
                    srvDesc.Texture1DArray.ArraySize,
                    srvDesc.Texture1DArray.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2D:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture2D.MostDetailedMip,
                    srvDesc.Texture2D.MipLevels,
                    srvDesc.Texture2D.PlaneSlice,
                    srvDesc.Texture2D.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture2DArray.MostDetailedMip,
                    srvDesc.Texture2DArray.MipLevels,
                    srvDesc.Texture2DArray.FirstArraySlice,
                    srvDesc.Texture2DArray.ArraySize,
                    srvDesc.Texture2DArray.PlaneSlice,
                    srvDesc.Texture2DArray.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2DMS:
//                hash_combine(seed, srvDesc.Texture2DMS.UnusedField_NothingToDefine);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture2DMSArray.FirstArraySlice,
                    srvDesc.Texture2DMSArray.ArraySize);
                break;
            case D3D12_SRV_DIMENSION_TEXTURE3D:
                seed = hash_combine_impl(seed, 
                    srvDesc.Texture3D.MostDetailedMip,
                    srvDesc.Texture3D.MipLevels,
                    srvDesc.Texture3D.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                seed = hash_combine_impl(seed, 
                    srvDesc.TextureCube.MostDetailedMip,
                    srvDesc.TextureCube.MipLevels,
                    srvDesc.TextureCube.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
                seed = hash_combine_impl(seed, 
                    srvDesc.TextureCubeArray.MostDetailedMip,
                    srvDesc.TextureCubeArray.MipLevels,
                    srvDesc.TextureCubeArray.First2DArrayFace,
                    srvDesc.TextureCubeArray.NumCubes,
                    srvDesc.TextureCubeArray.ResourceMinLODClamp);
                break;
            case D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE:
                hash_combine_impl(seed, srvDesc.RaytracingAccelerationStructure.Location);
                break;
            }

            return seed;
        }
    };

    template<>
    struct hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>
    {
        std::size_t operator()(const D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc) const noexcept
        {
            std::size_t seed = 0;

            seed = hash_combine_impl(seed, 
                uavDesc.Format, 
                uavDesc.ViewDimension);

            switch (uavDesc.ViewDimension)
            {
            case D3D12_UAV_DIMENSION_BUFFER:
                seed = hash_combine_impl(seed, 
                    uavDesc.Buffer.FirstElement,
                    uavDesc.Buffer.NumElements,
                    uavDesc.Buffer.StructureByteStride,
                    uavDesc.Buffer.CounterOffsetInBytes,
                    uavDesc.Buffer.Flags);
                break;
            case D3D12_UAV_DIMENSION_TEXTURE1D:
                seed = hash_combine_impl(seed, uavDesc.Texture1D.MipSlice);
                break;
            case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
                seed = hash_combine_impl(seed, 
                    uavDesc.Texture1DArray.MipSlice,
                    uavDesc.Texture1DArray.FirstArraySlice,
                    uavDesc.Texture1DArray.ArraySize);
                break;
            case D3D12_UAV_DIMENSION_TEXTURE2D:
                seed = hash_combine_impl(seed, 
                    uavDesc.Texture2D.MipSlice,
                    uavDesc.Texture2D.PlaneSlice);
                break;
            case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
                seed = hash_combine_impl(seed, 
                    uavDesc.Texture2DArray.MipSlice,
                    uavDesc.Texture2DArray.FirstArraySlice,
                    uavDesc.Texture2DArray.ArraySize,
                    uavDesc.Texture2DArray.PlaneSlice);
                break;
            case D3D12_UAV_DIMENSION_TEXTURE3D:
                seed = hash_combine(seed, 
                    uavDesc.Texture3D.MipSlice,
                    uavDesc.Texture3D.FirstWSlice,
                    uavDesc.Texture3D.WSize);
                break;
            }

            return seed;
        }
    };
} // namespace std