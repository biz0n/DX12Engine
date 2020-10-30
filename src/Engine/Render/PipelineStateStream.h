#pragma once

#include <Types.h>
#include <Hash.h>
#include <d3d12.h>

#include <d3dx12.h>

#include <compare>

namespace Engine::Render
{
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsvFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormats;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;

        auto operator<=>(const PipelineStateStream& other) const = default;
    };
}

namespace std
{
    template<>
    struct hash<Engine::Render::PipelineStateStream>
    {
        size_t operator()(const Engine::Render::PipelineStateStream& key) const
        {    
            return std::hash_combine(
                //key.rootSignature,
                key.inputLayout,
                key.primitiveTopologyType,
                key.VS,
                key.PS,
                key.dsvFormat,
                key.rtvFormats,
                key.rasterizer
               );
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE& key) const
        {    
            ID3D12RootSignature* desc = key;
            return std::hash<void*>{}(desc);
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT& key) const
        {    
            D3D12_INPUT_LAYOUT_DESC desc = key;
            return std::hash_combine(
                desc.NumElements,
                desc.pInputElementDescs );
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY& key) const
        {    
            D3D12_PRIMITIVE_TOPOLOGY_TYPE desc = key;
            return desc;
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT& key) const
        {    
            DXGI_FORMAT desc = key;
            return desc;
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER& key) const
        {    
            CD3DX12_RASTERIZER_DESC desc = key;
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
                desc.SlopeScaledDepthBias
            );
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS& key) const
        {    
            D3D12_RT_FORMAT_ARRAY desc = key;
            
            return std::hash_combine(
                desc.NumRenderTargets,
                std::hash_combine(desc.RTFormats)
            );
        }
    };

    template<>
    struct hash<D3D12_SHADER_BYTECODE>
    {
        size_t operator()(const D3D12_SHADER_BYTECODE& key) const
        {    
            return std::hash_combine(
                key.BytecodeLength,
                key.pShaderBytecode
            );
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_VS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_VS& key) const
        {    
            D3D12_SHADER_BYTECODE desc = key;
            
            return std::hash<D3D12_SHADER_BYTECODE>{}(desc);
        }
    };

    template<>
    struct hash<CD3DX12_PIPELINE_STATE_STREAM_PS>
    {
        size_t operator()(const CD3DX12_PIPELINE_STATE_STREAM_PS& key) const
        {    
            D3D12_SHADER_BYTECODE desc = key;
            
            return std::hash<D3D12_SHADER_BYTECODE>{}(desc);
        }
    };
}