#pragma once

#include <Types.h>
#include <Window.h>
#include <Graphics.h>
#include <Timer.h>
#include <Exceptions.h>
#include <ResourceStateTracker.h>

#include <d3dx12.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace Engine
{

    class IGame;

    class App
    {
    public:
        App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        ~App();
        int Run(SharedPtr<IGame> game);

        ComPtr<ID3D12GraphicsCommandList> GetCommandList();
        uint64 ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
        ComPtr<ID3D12Device2> GetDevice() { return graphics->GetDevice(); }
        void WaitForFenceValue(uint64);
        uint32 Present();
        uint32 GetCurrentBackBufferIndex() const { return mCurrentBackBufferIndex; }
        void Flush();

        int32 GetWidth() { return window->GetWidth(); }
        int32 GetHeight() { return window->GetHeight(); }

        ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE, uint32);

        ComPtr<ID3D12Resource> GetCurrentBackBuffer() { return mBackBuffers[mCurrentBackBufferIndex]; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView()
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                              mCurrentBackBufferIndex, mRTVDescriptorSize);
            return rtv;
        }

    private:
        void Init();

        void OnResize(int32, int32);
        void OnActiveChanged(bool);
        void OnKeyPressed(KeyEvent);
        void OnPaint();

        ComPtr<ID3D12CommandQueue> CreateCommandQueue(D3D12_COMMAND_LIST_TYPE);
        ComPtr<IDXGISwapChain4> CreateSwapChain();

        void UpdateBackBuffers(ComPtr<IDXGISwapChain>, ComPtr<ID3D12DescriptorHeap>);
        ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE);
        ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator>, D3D12_COMMAND_LIST_TYPE);
        ComPtr<ID3D12Fence> CreateFence();
        HANDLE CreateEventHandle();
        uint64 Signal(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, uint64 &);
        void WaitForFenceValue(ComPtr<ID3D12Fence>, HANDLE, uint64);
        void Flush(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, HANDLE, uint64 &);

    public:
        static const uint32 SwapChainBufferCount = 2;

    private:
        UniquePtr<Window> window;
        UniquePtr<Graphics> graphics;

    public:
        SharedPtr<GlobalResourceStateTracker> mGlobalResourceStateTracker;

        ComPtr<ID3D12CommandQueue> mDirectCommandQueue;

        ComPtr<IDXGISwapChain4> mSwapChain;
        ComPtr<ID3D12Resource> mBackBuffers[SwapChainBufferCount];
        ComPtr<ID3D12CommandAllocator> mCommandAllocators[SwapChainBufferCount];
        ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;

        uint32 mRTVDescriptorSize;

        ComPtr<ID3D12GraphicsCommandList> mCommandList;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        uint64 mFenceValue;

        uint32 mCurrentBackBufferIndex;

        Timer timer;

        SharedPtr<IGame> mGame;
    };

} // namespace Engine