#include "PipelineStateProvider.h"

#include <Exceptions.h>

#include <d3dx12.h>

namespace Engine::Render
{
    PipelineStateProvider::PipelineStateProvider(ComPtr<ID3D12Device2> device) : mDevice(device)
    {
    }

    PipelineStateProvider::~PipelineStateProvider() = default;

    ComPtr<ID3D12PipelineState> PipelineStateProvider::CreatePipelineState(const PipelineStateStream& pipelineStateStream)
    {
        size_t hash = std::hash<PipelineStateStream>{}(pipelineStateStream);

        auto iter = mPipelineStates.find(hash);
        if (iter != mPipelineStates.end())
        {
            return iter->second;
        }
        else
        {
            ComPtr<ID3D12PipelineState> pipelineState;    

            auto stream = pipelineStateStream;
            D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { 
                sizeof(PipelineStateStream), 
                &stream};

            ThrowIfFailed(mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)));

            mPipelineStates.emplace(hash, pipelineState);
            return pipelineState;
        }
    }
} // namespace Engine::Render