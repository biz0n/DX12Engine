#pragma once

#include <Types.h>
#include <Hash.h>
#include <Name.h>
#include <HAL/DirectXHashes.h>
#include <d3d12.h>

#include <d3dx12.h>

#include <vector>
#include <compare>


namespace Engine::Render
{
    struct PipelineStateProxy
    {
        Name rootSignatureName;
        std::string vertexShaderName;
        std::string pixelShaderName;
        std::string geometryShaderName;
        std::string meshShaderName;
        std::string amplificationShaderName;
        DXGI_FORMAT dsvFormat;
        std::vector<DXGI_FORMAT> rtvFormats;
        D3D12_RASTERIZER_DESC rasterizer;
        D3D12_DEPTH_STENCIL_DESC depthStencil;

        auto operator<=>(const PipelineStateProxy& other) const = default;
    };

    struct ComputePipelineStateProxy
    {
        Name rootSignatureName;
        std::string computeShaderName;

        auto operator<=>(const ComputePipelineStateProxy& other) const = default;
    };

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_GS GS;
        CD3DX12_PIPELINE_STATE_STREAM_AS AS;
        CD3DX12_PIPELINE_STATE_STREAM_MS MS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsvFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormats;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depthStencil;

        auto operator<=>(const PipelineStateStream& other) const = default;
    };

    struct ComputePipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_CS CS;

        auto operator<=>(const ComputePipelineStateStream& other) const = default;
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
                key.rootSignature,
                key.inputLayout,
                key.primitiveTopologyType,
                key.VS,
                key.PS,
                key.dsvFormat,
                key.rtvFormats,
                key.rasterizer,
                key.depthStencil
               );
        }
    };

    template<>
    struct hash<Engine::Render::PipelineStateProxy>
    {
        size_t operator()(const Engine::Render::PipelineStateProxy& key) const
        {    
            return std::hash_combine(
                key.rootSignatureName,
                key.vertexShaderName,
                key.pixelShaderName,
                key.geometryShaderName,
                key.dsvFormat,
                std::hash_combine(key.rtvFormats.begin(), key.rtvFormats.end()),
                key.rasterizer,
                key.depthStencil
               );
        }
    };

    template<>
    struct hash<Engine::Render::ComputePipelineStateStream>
    {
        size_t operator()(const Engine::Render::ComputePipelineStateStream& key) const
        {    
            return std::hash_combine(
                key.rootSignature,
                key.CS
               );
        }
    };

    template<>
    struct hash<Engine::Render::ComputePipelineStateProxy>
    {
        size_t operator()(const Engine::Render::ComputePipelineStateProxy& key) const
        {    
            return std::hash_combine(
                key.rootSignatureName,
                key.computeShaderName
               );
        }
    };
}