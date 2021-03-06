#pragma once

#include <Types.h>

#include <Name.h>
#include <Render/RenderForwards.h>

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

            ComPtr<ID3D12PipelineState> GetPipelineState(const Name& name);

        private:
            ComPtr<ID3D12PipelineState> CreatePipelineState(const PipelineStateStream& pipelineStateStream);

        private:
            ComPtr<ID3D12Device2> mDevice;
            ShaderProvider* mShaderProvider;
            RootSignatureProvider* mRootSignatureProvider;
            std::unordered_map<Name, ComPtr<ID3D12PipelineState>> mPipelineStates;

    };
}