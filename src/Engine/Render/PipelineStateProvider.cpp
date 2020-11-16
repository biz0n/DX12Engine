#include "PipelineStateProvider.h"

#include <Exceptions.h>

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

    ComPtr<ID3D12PipelineState> PipelineStateProvider::CreatePipelineState(const PipelineStateStream& pipelineStateStream)
    {
        ComPtr<ID3D12PipelineState> pipelineState;    

        auto stream = pipelineStateStream;
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
            sizeof(PipelineStateStream), 
            &stream};

        ThrowIfFailed(mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)));
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

        mPipelineStates.emplace(name, std::move(CreatePipelineState(stateStream)));
    }

    ComPtr<ID3D12PipelineState> PipelineStateProvider::GetPipelineState(const Name& name)
    {
        return mPipelineStates[name];
    }
} // namespace Engine::Render