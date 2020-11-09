#include "RenderContext.h"

#include <Exceptions.h>
#include <EventTracker.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Render/SwapChain.h>
#include <Memory/CommandAllocatorPool.h>
#include <Render/UIRenderContext.h>
#include <Render/CommandListUtils.h>
#include <Render/CommandQueue.h>
#include <Render/Graphics.h>
#include <Render/PipelineStateProvider.h>
#include <Render/ShaderProvider.h>

namespace Engine
{
    RenderContext::RenderContext(View view) : mFrameCount(0), mEventTracker{}
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

        for (auto frameIndex = 0; frameIndex < EngineConfig::SwapChainBufferCount; ++frameIndex)
        {
            mCommandAllocators[frameIndex] = MakeUnique<CommandAllocatorPool>(Device(), 0);
        }

        mUIRenderContext = MakeShared<UIRenderContext>(
            view,
            Device(),
            EngineConfig::SwapChainBufferCount,
            mSwapChain->GetCurrentBackBuffer()->GetDesc().Format);

            mPipelineStateProvider = MakeUnique<Render::PipelineStateProvider>(Device());
            mShaderProvider = MakeUnique<Render::ShaderProvider>();
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
        auto currentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        GetGraphicsCommandQueue()->WaitForFenceCPU(mFenceValues[currentBackBufferIndex]);

        for (uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
            GetDescriptorAllocator(type)->ReleaseStaleDescriptors(mFrameValues[currentBackBufferIndex]);
        }

        mCommandAllocators[currentBackBufferIndex]->Reset();

        ++mFrameCount;

        mUIRenderContext->BeginFrame();
    }

    void RenderContext::EndFrame()
    {       
        auto currentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
        auto resourceStateTracker = MakeShared<ResourceStateTracker>(mGlobalResourceStateTracker);

        auto uiCommandList = CreateGraphicsCommandList();
        uiCommandList->SetName(L"UI Render List");

        CommandListUtils::TransitionBarrier(resourceStateTracker, mSwapChain->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        auto rtv = mSwapChain->GetCurrentRenderTargetView();
        uiCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        //resourceStateTracker->FlushBarriers(uiCommandList);
        mUIRenderContext->Draw(uiCommandList);

        resourceStateTracker->FlushBarriers(uiCommandList);
        uiCommandList->Close();

        auto prePassCommandList = CreateGraphicsCommandList();

        auto barriers = resourceStateTracker->FlushPendingBarriers(prePassCommandList);
        resourceStateTracker->CommitFinalResourceStates();

        prePassCommandList->Close();
        

        std::vector<ID3D12CommandList*> commandLists;

        if (barriers > 0)
        {
            commandLists.push_back(prePassCommandList.Get());
        }
        commandLists.push_back(uiCommandList.Get());

        auto postPassCommandList = CreateGraphicsCommandList();
        CommandListUtils::TransitionBarrier(resourceStateTracker, mSwapChain->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
        resourceStateTracker->FlushPendingBarriers(postPassCommandList);
        resourceStateTracker->CommitFinalResourceStates();
        postPassCommandList->Close();

        commandLists.push_back(postPassCommandList.Get());


        mFenceValues[currentBackBufferIndex] = GetGraphicsCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());
        mFrameValues[currentBackBufferIndex] = GetFrameCount();

        currentBackBufferIndex = mSwapChain->Present();
    }

    void RenderContext::WaitForIdle()
    {
        GetComputeCommandQueue()->Flush();
        GetCopyCommandQueue()->Flush();
        GetGraphicsCommandQueue()->Flush();
    }
}