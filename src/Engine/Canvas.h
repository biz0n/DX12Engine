#pragma once

#include <Types.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine
{
    class RenderContext;


    class Canvas
    {
    public:
        Canvas(HWND hWnd, uint32 width, uint32 height, SharedPtr<RenderContext> renderContext);

        static const uint32 SwapChainBufferCount = 2;

        uint32 GetCurrentBackBufferIndex() const;
        ComPtr<ID3D12Resource> GetCurrentBackBuffer();
        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

        uint32 GetWidth() const { return mWidth; }
        uint32 GetHeight() const { return mHeight; }

        uint32 Present();

        void Resize(uint32 width, uint32 height);

    private:
        ComPtr<IDXGISwapChain4> CreateSwapChain();
        bool CheckTearing();
        void UpdateBackBuffers();

    private:
        bool mIsTearingSupported;
        ComPtr<IDXGISwapChain4> mSwapChain;
        ComPtr<ID3D12Resource> mBackBuffers[SwapChainBufferCount];
        DescriptorAllocation mBackBufferAllocation;

        uint32 mCurrentBackBufferIndex;

        SharedPtr<RenderContext> mRenderContext;
        

        uint32 mWidth;
        uint32 mHeight;
        HWND mHWnd;
    };
} // namespace Engine