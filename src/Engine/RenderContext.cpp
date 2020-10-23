#include "RenderContext.h"

#include <Exceptions.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <SwapChain.h>
#include <CommandAllocatorPool.h>
#include <UIRenderContext.h>
#include <CommandListUtils.h>
#include <CommandQueue.h>
#include <Graphics.h>

namespace Engine
{
    RenderContext::RenderContext(View view) : mFrameCount(0)
    {
        mGraphics = MakeUnique<Graphics>();

        for (uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
            uint32 incrementalSize = Device()->GetDescriptorHandleIncrementSize(type);
            mDescriptorAllocators[i] = MakeShared<DescriptorAllocator>(Device(), type);
        }

        mDirrectCommandQueue = MakeShared<CommandQueue>(Device(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        mComputeCommandQueue = MakeShared<CommandQueue>(Device(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
        mCopyCommandQueue = MakeShared<CommandQueue>(Device(), D3D12_COMMAND_LIST_TYPE_COPY);

        mDirrectCommandQueue->D3D12CommandQueue()->SetName(L"Render Queue");
        mComputeCommandQueue->D3D12CommandQueue()->SetName(L"Compute Queue");
        mCopyCommandQueue->D3D12CommandQueue()->SetName(L"Copy Queue");

        mGlobalResourceStateTracker = MakeUnique<GlobalResourceStateTracker>();

        mSwapChain = MakeShared<SwapChain>(
            view,
            mGraphics.get(),
            mGlobalResourceStateTracker,
            GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV), mDirrectCommandQueue->D3D12CommandQueue());

        for (auto frameIndex = 0; frameIndex < SwapChain::SwapChainBufferCount; ++frameIndex)
        {
            mCommandAllocators[frameIndex] = MakeUnique<CommandAllocatorPool>(Device(), 3);
            mResourceStateTrackers[frameIndex] = MakeShared<ResourceStateTracker>(mGlobalResourceStateTracker);
        }

        mUIRenderContext = MakeShared<UIRenderContext>(
            view,
            Device(),
            SwapChain::SwapChainBufferCount,
            mSwapChain->GetCurrentBackBuffer()->GetDesc().Format);
    }

    RenderContext::~RenderContext() = default;

    ComPtr<ID3D12Device2> RenderContext::Device() const
    {
        return mGraphics->GetDevice();
    }

    ComPtr<IDXGIFactory4> RenderContext::GIFactory() const
    {
        return mGraphics->GetGIFactory();
    }

    SharedPtr<CommandQueue> RenderContext::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
    {
        switch (type)
        {
            case D3D12_COMMAND_LIST_TYPE_DIRECT:
                return mDirrectCommandQueue;
            case D3D12_COMMAND_LIST_TYPE_COMPUTE:
                return mComputeCommandQueue;
            case D3D12_COMMAND_LIST_TYPE_COPY:
                return mCopyCommandQueue;
        }

        assert(false && "Invalid command queue type.");
        return nullptr;
    }

    ComPtr<ID3D12GraphicsCommandList> RenderContext::CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
    {
        auto currentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        auto allocator = mCommandAllocators[currentBackBufferIndex]->GetNextAllocator(type);

        ComPtr<ID3D12GraphicsCommandList> commandList;

        ThrowIfFailed(Device()->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

        ThrowIfFailed(commandList->Close());

        commandList->Reset(allocator.Get(), nullptr);

        return commandList;
    }

    void RenderContext::BeginFrame()
    {
        ++mFrameCount;

        auto currentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        mCommandAllocators[currentBackBufferIndex]->Reset();

        auto resourceStateTracker = mResourceStateTrackers[currentBackBufferIndex];
        CommandListUtils::TransitionBarrier(resourceStateTracker, mSwapChain->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        mUIRenderContext->BeginFrame();
    }

    void RenderContext::EndFrame()
    {
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
            
        auto currentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
        auto resourceStateTracker = mResourceStateTrackers[currentBackBufferIndex];

        auto uiCommandList = CreateGraphicsCommandList();
        uiCommandList->SetName(L"UI Render List");

        auto rtv = mSwapChain->GetCurrentRenderTargetView();
        uiCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        resourceStateTracker->FlushBarriers(uiCommandList);
        mUIRenderContext->Draw(uiCommandList);

        CommandListUtils::TransitionBarrier(resourceStateTracker, mSwapChain->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);

        resourceStateTracker->FlushBarriers(uiCommandList);

        mFenceValues[currentBackBufferIndex] = GetGraphicsCommandQueue()->ExecuteCommandList(uiCommandList);
        mFrameValues[currentBackBufferIndex] = GetFrameCount();

        currentBackBufferIndex = mSwapChain->Present();

        GetGraphicsCommandQueue()->WaitForFenceCPU(mFenceValues[currentBackBufferIndex]);

        for (uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
            GetDescriptorAllocator(type)->ReleaseStaleDescriptors(mFrameValues[currentBackBufferIndex]);
        }
    }
}