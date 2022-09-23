#include "PipelineStateProvider.h"

#include <Exceptions.h>

#include <Render/PipelineStateStream.h>
#include <Render/RootSignatureProvider.h>
#include <Render/ShaderProvider.h>

#include <HAL/ShaderCreationInfo.h>
#include <HAL/RootSignature.h>
#include <HAL/DirectXExtensions.h>

#include <d3dx12.h>

namespace Engine::Render
{
    PipelineStateProvider::PipelineStateProvider(ComPtr<ID3D12Device2> device, ShaderProvider* shaderProvider, RootSignatureProvider* rootSignatureProvider)
     : mDevice{device}, mShaderProvider{shaderProvider}, mRootSignatureProvider{rootSignatureProvider}
    {
    }

    PipelineStateProvider::~PipelineStateProvider() = default;

    ComPtr<ID3D12PipelineState> PipelineStateProvider::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStream)
    {
        ComPtr<ID3D12PipelineState> pipelineState;    

        ThrowIfFailed(mDevice->CreatePipelineState(&pipelineStateStream, IID_PPV_ARGS(&pipelineState)));
        return pipelineState;
    }

    void PipelineStateProvider::CreatePipelineState(const Name& name, const PipelineStateProxy& pipelineStateProxy)
    {
        if (mPipelineStates.contains(name))
        {
            return;
        }

        D3D12_RT_FORMAT_ARRAY rtFormats = {};
        rtFormats.NumRenderTargets = static_cast<uint32>(pipelineStateProxy.rtvFormats.size());
        for (auto i = 0; i < pipelineStateProxy.rtvFormats.size(); ++i)
        {
            rtFormats.RTFormats[i] = pipelineStateProxy.rtvFormats.at(i);
        }

        PipelineStateStream stateStream;
        stateStream.depthStencil = CD3DX12_DEPTH_STENCIL_DESC(pipelineStateProxy.depthStencil);
        stateStream.dsvFormat = pipelineStateProxy.dsvFormat;
        if (!pipelineStateProxy.vertexShaderName.empty())
        {
            stateStream.VS = mShaderProvider->GetShader({ pipelineStateProxy.vertexShaderName, "mainVS", "vs_6_6" });
        }
        
        if (!pipelineStateProxy.pixelShaderName.empty())
        {
            stateStream.PS = mShaderProvider->GetShader({ pipelineStateProxy.pixelShaderName, "mainPS", "ps_6_6" });
        }
        if (!pipelineStateProxy.geometryShaderName.empty())
        {
            stateStream.GS = mShaderProvider->GetShader({pipelineStateProxy.geometryShaderName, "mainGS", "gs_6_6"});
        }
        if (!pipelineStateProxy.meshShaderName.empty())
        {
            stateStream.MS = mShaderProvider->GetShader({ pipelineStateProxy.meshShaderName, "mainMS", "ms_6_6" });
        }
        if (!pipelineStateProxy.amplificationShaderName.empty())
        {
            stateStream.AS = mShaderProvider->GetShader({ pipelineStateProxy.amplificationShaderName, "mainAS", "as_6_6" });
        }
        stateStream.rasterizer = CD3DX12_RASTERIZER_DESC(pipelineStateProxy.rasterizer);
        stateStream.rootSignature = mRootSignatureProvider->GetRootSignature(pipelineStateProxy.rootSignatureName)->GetD3D12RootSignature().Get();
        stateStream.rtvFormats = rtFormats;
        stateStream.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
            sizeof(PipelineStateStream), 
            &stateStream};

        auto pipelineState = CreatePipelineState(pipelineStateStreamDesc);
        HAL::SetResourceName(pipelineState, name.string());

        mPipelineStates.emplace(name, std::move(pipelineState));
        mRootSignatureMap.emplace(name, pipelineStateProxy.rootSignatureName);
    }

    void PipelineStateProvider::CreatePipelineState(const Name& name, const ComputePipelineStateProxy& pipelineStateProxy)
    {
        if (mPipelineStates.contains(name))
        {
            return;
        }

        ComputePipelineStateStream stateStream;
        stateStream.rootSignature = mRootSignatureProvider->GetRootSignature(pipelineStateProxy.rootSignatureName)->GetD3D12RootSignature().Get();
        stateStream.CS = mShaderProvider->GetShader({pipelineStateProxy.computeShaderName, "mainCS", "cs_6_6"});

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
            sizeof(ComputePipelineStateStream), 
            &stateStream};

        auto pipelineState = CreatePipelineState(pipelineStateStreamDesc);
        HAL::SetResourceName(pipelineState, name.string());

        mPipelineStates.emplace(name, std::move(pipelineState));
        mRootSignatureMap.emplace(name, pipelineStateProxy.rootSignatureName);
    }

    ComPtr<ID3D12PipelineState> PipelineStateProvider::GetPipelineState(const Name& name)
    {
        return mPipelineStates[name];
    }

    const Name& PipelineStateProvider::GetAssociatedRootSignatureName(const Name& pipelineName) const
    {
        return mRootSignatureMap.at(pipelineName);
    }
} // namespace Engine::Render