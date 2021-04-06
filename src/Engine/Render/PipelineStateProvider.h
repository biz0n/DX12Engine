#pragma once

#include <Types.h>

#include <Name.h>
#include <Render/RenderForwards.h>
#include <HAL/HALForwards.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class PipelineStateProvider
    {
        public:
            PipelineStateProvider(ComPtr<ID3D12Device2> device, ShaderProvider* shaderProvider, RootSignatureProvider* rootSignatureProvider);
            ~PipelineStateProvider();

            void CreatePipelineState(const Name& name, const PipelineStateProxy& pipelineStateProxy);
            void CreatePipelineState(const Name& name, const ComputePipelineStateProxy& pipelineStateProxy);

            ComPtr<ID3D12PipelineState> GetPipelineState(const Name& name);

            const Name& GetAssociatedRootSignatureName(const Name& pipelineName) const;

        private:
            ComPtr<ID3D12PipelineState> CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStream);

        private:
            ComPtr<ID3D12Device2> mDevice;
            ShaderProvider* mShaderProvider;
            Render::RootSignatureProvider* mRootSignatureProvider;
            std::unordered_map<Name, ComPtr<ID3D12PipelineState>> mPipelineStates;
            std::unordered_map<Name, Name> mRootSignatureMap;
    };
}