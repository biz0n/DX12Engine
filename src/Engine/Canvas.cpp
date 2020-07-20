#include "Canvas.h"

#include <RenderContext.h>
#include <Exceptions.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine
{
    Canvas::Canvas(HWND hWnd, uint32 width, uint32 height, SharedPtr<RenderContext> renderContext)
        : mHWnd(hWnd)
        , mWidth(width)
        , mHeight(height)
        , mRenderContext(renderContext)
        , mCurrentBackBufferIndex(0)
    {
        mIsTearingSupported = CheckTearing();
        mSwapChain = CreateSwapChain();
        UpdateBackBuffers();
    }

    uint32 Canvas::GetCurrentBackBufferIndex() const
    {
        return mCurrentBackBufferIndex;
    }

    ComPtr<ID3D12Resource> Canvas::GetCurrentBackBuffer()
    {
        return mBackBuffers[mCurrentBackBufferIndex];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Canvas::GetCurrentRenderTargetView() const
    {
        return mBackBufferAllocation.GetDescriptor(mCurrentBackBufferIndex);
    }

    uint32 Canvas::Present()
    {
        bool vSync = false;
        int32 syncInterval = vSync ? 1 : 0;
        int32 presentFlags = mIsTearingSupported && !vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));

        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        return mCurrentBackBufferIndex;
    }

    void Canvas::Resize(uint32 width, uint32 height)
    {
        mWidth = width;
        mHeight = height;

        for (uint32 i = 0; i < SwapChainBufferCount; ++i)
        {
            mRenderContext->GetResourceStateTracker()->UntrackResource(mBackBuffers[i].Get());
            mBackBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC desc = {};
        ThrowIfFailed(mSwapChain->GetDesc(&desc));
        ThrowIfFailed(mSwapChain->ResizeBuffers(SwapChainBufferCount, width, height, desc.BufferDesc.Format, desc.Flags));

        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        UpdateBackBuffers();
    }

    ComPtr<IDXGISwapChain4> Canvas::CreateSwapChain()
    {
        ComPtr<IDXGISwapChain> swapChain;

        DXGI_SWAP_CHAIN_DESC sd;
        sd.BufferDesc.Width = mWidth;
        sd.BufferDesc.Height = mHeight;

        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        sd.SampleDesc = {1, 0};

        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = SwapChainBufferCount;
        sd.Windowed = true;

        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.OutputWindow = mHWnd;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | (mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        ThrowIfFailed(mRenderContext->GIFactory()->CreateSwapChain(mRenderContext->GetCommandQueue().Get(), &sd, &swapChain));

        ThrowIfFailed(mRenderContext->GIFactory()->MakeWindowAssociation(mHWnd, DXGI_MWA_NO_ALT_ENTER));

        ComPtr<IDXGISwapChain4> swapChain4;

        ThrowIfFailed(swapChain.As(&swapChain4));

        return swapChain4;
    }

    void Canvas::UpdateBackBuffers()
    {
        mBackBufferAllocation = mRenderContext->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChainBufferCount);
        for (uint32 i = 0; i < SwapChainBufferCount; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

            auto rtvHandle = mBackBufferAllocation.GetDescriptor(i);
            mRenderContext->Device()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

            mBackBuffers[i] = backBuffer;

            mRenderContext->GetResourceStateTracker()->TrackResource(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);
        }
    }

    bool Canvas::CheckTearing()
    {
        BOOL allowTearing = FALSE;

        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(mRenderContext->GIFactory().As(&factory5)))
        {
            ThrowIfFailed(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));
        }

        return allowTearing == TRUE;
    }

} // namespace Engine