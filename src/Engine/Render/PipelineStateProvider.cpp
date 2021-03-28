#include "PipelineStateProvider.h"

#include <Exceptions.h>
#include <StringUtils.h>

#include <Render/PipelineStateStream.h>
#include <Render/RootSignatureProvider.h>
#include <Render/ShaderProvider.h>
#include <Render/ShaderCreationInfo.h>
#include <Render/RootSignature.h>

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
        stateStream.inputLayout = {pipelineStateProxy.inputLayout.data(), static_cast<uint32>(pipelineStateProxy.inputLayout.size())};
        stateStream.primitiveTopologyType = pipelineStateProxy.primitiveTopologyType;
        stateStream.PS = CD3DX12_SHADER_BYTECODE(mShaderProvider->GetShader({pipelineStateProxy.pixelShaderName, "mainPS", "ps_5_1"}).Get());
        stateStream.VS = CD3DX12_SHADER_BYTECODE(mShaderProvider->GetShader({pipelineStateProxy.vertexShaderName, "mainVS", "vs_5_1"}).Get());
        stateStream.rasterizer = CD3DX12_RASTERIZER_DESC(pipelineStateProxy.rasterizer);
        stateStream.rootSignature = mRootSignatureProvider->GetRootSignature(pipelineStateProxy.rootSignatureName)->GetD3D12RootSignature().Get();
        stateStream.rtvFormats = rtFormats;

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
            sizeof(PipelineStateStream), 
            &stateStream};

        auto pipelineState = CreatePipelineState(pipelineStateStreamDesc);
        pipelineState->SetName(StringToWString(name.string()).c_str());

        mPipelineStates.emplace(name, std::move(pipelineState));
    }

    void PipelineStateProvider::CreatePipelineState(const Name& name, const ComputePipelineStateProxy& pipelineStateProxy)
    {
        if (mPipelineStates.contains(name))
        {
            return;
        }

        ComputePipelineStateStream stateStream;
        stateStream.rootSignature = mRootSignatureProvider->GetRootSignature(pipelineStateProxy.rootSignatureName)->GetD3D12RootSignature().Get();
        stateStream.CS = CD3DX12_SHADER_BYTECODE(mShaderProvider->GetShader({pipelineStateProxy.computeShaderName, "mainCS", "cs_5_1"}).Get());

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
            sizeof(ComputePipelineStateStream), 
            &stateStream};

        auto pipelineState = CreatePipelineState(pipelineStateStreamDesc);
        pipelineState->SetName(StringToWString(name.string()).c_str());

        mPipelineStates.emplace(name, std::move(pipelineState));
    }

    ComPtr<ID3D12PipelineState> PipelineStateProvider::GetPipelineState(const Name& name)
    {
        return mPipelineStates[name];
    }
} // namespace Engine::Render