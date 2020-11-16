#pragma once

#include <Types.h>

#include <View.h>
#include <EngineConfig.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine
{
    class Graphics;
    struct View;
    class GlobalResourceStateTracker;


    class SwapChain
    {
    public:
        SwapChain(View view, const Graphics* graphis, SharedPtr<GlobalResourceStateTracker> resourceStateTracker, SharedPtr<Memory::DescriptorAllocator> rtvDescriptorAllocator, ID3D12CommandQueue* commandQueue);
        ~SwapChain();

        uint32 GetCurrentBackBufferIndex() const;
        ComPtr<ID3D12Resource> GetCurrentBackBuffer();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

        uint32 GetWidth() const { return mWidth; }
        uint32 GetHeight() const { return mHeight; }

        uint32 Present();

        void Resize(uint32 width, uint32 height);

    private:
        ComPtr<IDXGISwapChain4> CreateSwapChain(ID3D12CommandQueue* commandQueue);
        bool CheckTearing();
        void UpdateBackBuffers();

    private:
        bool mIsTearingSupported;
        ComPtr<IDXGISwapChain4> mSwapChain;
        ComPtr<ID3D12Resource> mBackBuffers[EngineConfig::SwapChainBufferCount];
        Memory::DescriptorAllocation mBackBufferAllocation;

        SharedPtr<GlobalResourceStateTracker> mResourceStateTracker;

        uint32 mCurrentBackBufferIndex;

        const Graphics* mGraphics;
        SharedPtr<Memory::DescriptorAllocator> mRTVDescriptorAllocator;

        uint32 mWidth;
        uint32 mHeight;
        View mView;
    };
} // namespace Engine