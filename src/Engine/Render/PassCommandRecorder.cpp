#include "PassCommandRecorder.h"

#include <Render/RenderContext.h>
#include <Render/SwapChain.h>
#include <Render/FrameResourceProvider.h>
#include <Render/CommandListUtils.h>
#include <Render/Texture.h>
#include <Render/RootSignatureProvider.h>
#include <Render/FrameTransientContext.h>
#include <Render/PipelineStateProvider.h>

#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DynamicDescriptorHeap.h>

#include <d3dx12.h>

namespace Engine::Render
{
    PassCommandRecorder::PassCommandRecorder(
        ComPtr<ID3D12GraphicsCommandList> commandList,
        ResourceStateTracker *resourceStateTracker,
        RenderContext *renderContext,
        const FrameResourceProvider *frameResourceProvider,
        FrameTransientContext* frameTransientContext) : mCommandList(commandList),
                                                        mResourceStateTracker(resourceStateTracker),
                                                        mRenderContext(renderContext),
                                                        mFrameResourceProvider(frameResourceProvider),
                                                        mFrameTransientContext(frameTransientContext)
    {
    }

    PassCommandRecorder::~PassCommandRecorder() = default;

    void PassCommandRecorder::SetViewPort(uint32 width, uint32 height)
    {
        if (width == 0 || height == 0)
        {
            width = mRenderContext->GetSwapChain()->GetWidth();
            height = mRenderContext->GetSwapChain()->GetHeight();
        }

        auto screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        auto scissorRect = CD3DX12_RECT(0, 0, width, height);

        mCommandList->RSSetViewports(1, &screenViewport);
        mCommandList->RSSetScissorRects(1, &scissorRect);
    }

    void PassCommandRecorder::SetRenderTargets(std::vector<Name> renderTargets, const Name& depthStencil)
    {
        SharedPtr<ResourceStateTracker> stateTrackerSharedPtr(mResourceStateTracker, [](ResourceStateTracker const*){}); //temporary solution

        D3D12_CPU_DESCRIPTOR_HANDLE dsDescriptor {0};

        if (depthStencil.isValid())
        {
            Render::Texture* dsTexture = mFrameResourceProvider->GetTexture(depthStencil);
            
            CommandListUtils::TransitionBarrier(stateTrackerSharedPtr, dsTexture->D3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

            dsDescriptor = dsTexture->GetDSDescriptor(mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).get());
        }

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
        renderTargetDescriptors.reserve(renderTargets.size());

        for (auto renderTargetName : renderTargets)
        {
            Render::Texture* rtTexture = mFrameResourceProvider->GetTexture(renderTargetName);

            CommandListUtils::TransitionBarrier(stateTrackerSharedPtr, rtTexture->D3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            auto rtDescriptor = rtTexture->GetRTDescriptor(mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).get());

            renderTargetDescriptors.push_back(rtDescriptor);
        }

        mCommandList->OMSetRenderTargets(static_cast<uint32>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), false, ((dsDescriptor.ptr != 0) ? &dsDescriptor : nullptr));
    }

    void PassCommandRecorder::SetBackBufferAsRenderTarget()
    {
        SharedPtr<ResourceStateTracker> stateTrackerSharedPtr(mResourceStateTracker, [](ResourceStateTracker const*){}); //temporary solution

        auto backBufferTexture = mRenderContext->GetSwapChain()->GetCurrentBackBufferTexture();
        auto backBuffer = backBufferTexture->D3D12ResourceCom();
        CommandListUtils::TransitionBarrier(stateTrackerSharedPtr, backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        auto rtv = backBufferTexture->GetRTDescriptor(mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).get());

        mCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    }

    void PassCommandRecorder::ClearRenderTargets(std::vector<Name> renderTargets)
    {
        float32 clearColor[] = {0};

        for (auto renderTargetName : renderTargets)
        {
            Render::Texture* rtTexture = mFrameResourceProvider->GetTexture(renderTargetName);

            auto rtDescriptor = rtTexture->GetRTDescriptor(mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).get());

            mCommandList->ClearRenderTargetView(rtDescriptor, clearColor, 0, nullptr);
        }
    }

    void PassCommandRecorder::ClearDepthStencil(const Name& depthStencil)
    {
        Render::Texture* dsTexture = mFrameResourceProvider->GetTexture(depthStencil);
        auto dsDescriptor = dsTexture->GetDSDescriptor(mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).get());

        mCommandList->ClearDepthStencilView(dsDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
    }
    
    void PassCommandRecorder::SetRootSignature(const Name& rootSignature)
    {
        if (mLastRootSignature == rootSignature)
        {
            return;
        }

        auto rs = mRenderContext->GetRootSignatureProvider()->GetRootSignature(rootSignature);
        mCommandList->SetGraphicsRootSignature(rs->GetD3D12RootSignature().Get());

        mFrameTransientContext->dynamicDescriptorHeap->ParseRootSignature(rs);
        mLastRootSignature = rootSignature;
    }

    void PassCommandRecorder::SetPipelineState(const Name& pso)
    {
        if (mLastPSO == pso)
        {
            return;
        }

        mCommandList->SetPipelineState(mRenderContext->GetPipelineStateProvider()->GetPipelineState(pso).Get());
        mLastPSO = pso;
    }

} // namespace Engine::Render