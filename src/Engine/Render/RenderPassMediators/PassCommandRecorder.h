#pragma once

#include <Types.h>
#include <Name.h>

#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{
    class PassCommandRecorder
    {
    public:
        PassCommandRecorder(
            ComPtr<ID3D12GraphicsCommandList> commandList,
            Memory::ResourceStateTracker *resourceStateTracker,
            RenderContext *renderContext,
            const FrameResourceProvider *frameResourceProvider,
            FrameTransientContext* frameTransientContext);
        ~PassCommandRecorder();

        ComPtr<ID3D12GraphicsCommandList> GetD3D12CommandList() const { return mCommandList; }

        void SetViewPort(uint32 width = 0, uint32 height = 0);

        void SetRenderTargets(std::vector<Name> renderTargets, const Name& depthStencil);
        void SetBackBufferAsRenderTarget();

        void ClearRenderTargets(std::vector<Name> renderTargets);
        void ClearDepthStencil(const Name& depthStencil);

        void SetRootSignature(const Name& rootSignature);

        void SetPipelineState(const Name& pso);

    private:
        ComPtr<ID3D12GraphicsCommandList> mCommandList;
        Memory::ResourceStateTracker *mResourceStateTracker;
        RenderContext *mRenderContext;
        const FrameResourceProvider *mFrameResourceProvider;
        FrameTransientContext* mFrameTransientContext;

        Name mLastRootSignature;
        Name mLastPSO;
    };

} // namespace Engine::Render
