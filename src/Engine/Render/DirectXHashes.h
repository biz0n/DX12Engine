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
} // namespace std