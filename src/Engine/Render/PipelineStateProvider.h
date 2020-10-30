#pragma once

#include <Types.h>

#include <Render/PipelineStateStream.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class PipelineStateProvider
    {
        public:
            PipelineStateProvider(ComPtr<ID3D12Device2> device);
            ~PipelineStateProvider();

            ComPtr<ID3D12PipelineState> CreatePipelineState(const PipelineStateStream& pipelineStateStream);

        private:
            ComPtr<ID3D12Device2> mDevice;
            std::unordered_map<size_t, ComPtr<ID3D12PipelineState>> mPipelineStates;

    };
}