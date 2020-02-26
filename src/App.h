#pragma once

#include "Types.h"
#include "Window.h"
#include "Graphics.h"
#include "Timer.h"
#include "Exceptions.h"

#include "d3dx12.h"
#include <wrl.h>

using namespace Microsoft::WRL;

class App
{
public:
    App();
    App(const App &) = delete;
    App &operator=(const App &) = delete;
    ~App();
    int Run();

private:
    void Init();
    void Update(const Timer &);

    void OnResize(int32, int32);
    void OnActiveChanged(bool);

    ComPtr<ID3D12CommandQueue> CreateCommandQueue(D3D12_COMMAND_LIST_TYPE);
    ComPtr<IDXGISwapChain> CreateSwapChain();
    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE, uint32);
    void UpdateBackBuffers(ComPtr<IDXGISwapChain>, ComPtr<ID3D12DescriptorHeap>);
    ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE);
    ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator>, D3D12_COMMAND_LIST_TYPE);
    ComPtr<ID3D12Fence> CreateFence();
    HANDLE CreateEventHandle();
    uint64 Signal(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, uint64 &);
    void WaitForFenceValue(ComPtr<ID3D12Fence>, HANDLE, uint64);
    void Flush(ComPtr<ID3D12CommandQueue>, ComPtr<ID3D12Fence>, HANDLE, uint64 &);

private:
    static const uint32 SwapChainBufferCount = 2;

private:
    UniquePtr<Window> window;
    UniquePtr<Graphics> graphics;

private:
    ComPtr<ID3D12CommandQueue> mDirectCommandQueue;

    ComPtr<IDXGISwapChain> mSwapChain;
    ComPtr<ID3D12Resource> mBackBuffers[SwapChainBufferCount];
    ComPtr<ID3D12CommandAllocator> mCommandAllocators[SwapChainBufferCount];
    ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
    ComPtr< ID3D12DescriptorHeap> mSRVDescriptorHeap;
    ComPtr< ID3D12DescriptorHeap> mDSVDescriptorHeap;
    uint32 mRTVDescriptorSize;

    ComPtr<ID3D12GraphicsCommandList> mCommandList;

    ComPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent;
    uint64 mFenceValue;
    uint64 mFenceValues[SwapChainBufferCount];

    uint32 mCurrentBackBufferIndex;

    Timer timer;
};