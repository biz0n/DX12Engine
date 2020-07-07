#include "App.h"
#include <Utils.h>
#include <Game.h>
#include <RenderContext.h>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>

namespace Engine
{
    App::App(SharedPtr<RenderContext> renderContext, SharedPtr<Canvas> canvas)
     : mRenderContext(renderContext), mCanvas(canvas), mFenceValue(0)
    {
    }

    App::~App()
    {
    }

    void App::Init(SharedPtr<Game> game)
    {
        mGame = game;

        timer.Reset();

        for (auto i = 0; i < Canvas::SwapChainBufferCount; ++i)
        {
            mCommandAllocators[i] = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
        }
        auto currentBackBufferIndex = mCanvas->GetCurrentBackBufferIndex();
        mCommandList = CreateCommandList(mCommandAllocators[currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

        mFence = CreateFence();
        mFenceEvent = CreateEventHandle();
    }

    ComPtr<ID3D12GraphicsCommandList> App::GetCommandList()
    {
        auto currentBackBufferIndex = mCanvas->GetCurrentBackBufferIndex();
        mCommandAllocators[currentBackBufferIndex]->Reset();

        mCommandList->Reset(mCommandAllocators[currentBackBufferIndex].Get(), nullptr);

        return mCommandList;
    }

    uint64 App::ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        ThrowIfFailed(commandList->Close());

        ID3D12CommandList *const commandLists[] = {commandList.Get()};
        mRenderContext->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

        return Signal(mRenderContext->GetCommandQueue(), mFence, mFenceValue);
    }

    void App::WaitForFenceValue(uint64 fenceValue)
    {
        WaitForFenceValue(mFence, mFenceEvent, fenceValue);
    }

    void App::Flush()
    {
        Flush(mRenderContext->GetCommandQueue(), mFence, mFenceEvent, mFenceValue);
    }

    void App::OnPaint()
    {
        timer.Tick();

        if (timer.IsPaused())
        {
            Sleep(16);
        }
        else
        {
            mGame->Update(timer);
            mGame->Draw(timer);
        }
    }

    void App::OnResize(int32 width, int32 height)
    {
        std::cout << "Width: " << width << " Height: " << height << std::endl;

        Flush(mRenderContext->GetCommandQueue(), mFence, mFenceEvent, mFenceValue);

        ImGui_ImplDX12_InvalidateDeviceObjects();

        //App::UpdateBackBuffers(mSwapChain, mRTVDescriptorHeap);

        ///  mScreenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

        //  mScissorRect = CD3DX12_RECT(0, 0, width, height);

        // ResizeDepthBuffer(width, height);

        ImGui_ImplDX12_CreateDeviceObjects();

        mGame->Resize(width, height);
    }

    void App::OnActiveChanged(bool isActive)
    {
        if (isActive)
        {
            timer.Start();
        }
        else
        {
            timer.Stop();
        }
    }

    void App::OnKeyPressed(KeyEvent keyEvent)
    {
        mGame->KeyPressed(keyEvent);
    }

    ComPtr<ID3D12CommandAllocator> App::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> allocator;

        ThrowIfFailed(mRenderContext->Device()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));

        return allocator;
    }

    ComPtr<ID3D12GraphicsCommandList> App::CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12GraphicsCommandList> commandList;

        ThrowIfFailed(mRenderContext->Device()->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

        ThrowIfFailed(commandList->Close());

        return commandList;
    }

    ComPtr<ID3D12Fence> App::CreateFence()
    {
        ComPtr<ID3D12Fence> fence;

        ThrowIfFailed(mRenderContext->Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        return fence;
    }

    HANDLE App::CreateEventHandle()
    {
        HANDLE fenceEvent;

        fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        return fenceEvent;
    }

    uint64 App::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64 &fenceValue)
    {
        auto fenceValueForSignal = ++fenceValue;

        ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

        return fenceValueForSignal;
    }

    void App::WaitForFenceValue(ComPtr<ID3D12Fence> fence, HANDLE fenceEvent, uint64 fenceValue)
    {
        if (fence->GetCompletedValue() < fenceValue)
        {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            ::WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    void App::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, HANDLE fenceEvent, uint64 &fenceValue)
    {
        uint64 fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
        WaitForFenceValue(fence, fenceEvent, fenceValueForSignal);
    }

} // namespace Engine