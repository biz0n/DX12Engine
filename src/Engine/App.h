#pragma once

#include <Types.h>
#include <Window.h>
#include <Graphics.h>
#include <Timer.h>
#include <Exceptions.h>
#include <ResourceStateTracker.h>
#include <Canvas.h>

#include <d3dx12.h>
#include <DirectXMath.h>

namespace Engine
{
    class Game;
    class RenderContext;

    class App
    {
    public:
        App(SharedPtr<RenderContext> renderContext, SharedPtr<Canvas> canvas);
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        ~App();

        ComPtr<ID3D12GraphicsCommandList> GetCommandList();
        uint64 ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
        void WaitForFenceValue(uint64);
        void Flush();

    public:
        void Init(SharedPtr<Game> game);

        void OnResize(int32, int32);
        void OnActiveChanged(bool);
        void OnKeyPressed(KeyEvent);
        void OnPaint();

    private:
        ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE);
        ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator>, D3D12_COMMAND_LIST_TYPE);
        ComPtr<ID3D12Fence> CreateFence();
        HANDLE CreateEventHandle();
        uint64 Signal(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, uint64 &);
        void WaitForFenceValue(ComPtr<ID3D12Fence>, HANDLE, uint64);
        void Flush(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, HANDLE, uint64 &);

    public:

    public:
        ComPtr<ID3D12CommandAllocator> mCommandAllocators[Canvas::SwapChainBufferCount];

        ComPtr<ID3D12GraphicsCommandList> mCommandList;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        uint64 mFenceValue;

        Timer timer;

        SharedPtr<Game> mGame;

        SharedPtr<RenderContext> mRenderContext;
        SharedPtr<Canvas> mCanvas;
    };

} // namespace Engine