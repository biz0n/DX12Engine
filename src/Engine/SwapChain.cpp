#include "SwapChain.h"

#include <View.h>
#include <Graphics.h>
#include <Exceptions.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <ResourceStateTracker.h>

namespace Engine
{
    SwapChain::SwapChain(
        View view,
        const Graphics* graphics,
        SharedPtr<GlobalResourceStateTracker> resourceStateTracker,
        SharedPtr<DescriptorAllocator> rtvDescriptorAllocator,
        ID3D12CommandQueue* commandQueue)
        : mView(view)
        , mWidth(view.width)
        , mHeight(view.height)
        , mGraphics(graphics)
        , mResourceStateTracker(resourceStateTracker)
        , mRTVDescriptorAllocator(rtvDescriptorAllocator)
        , mCurrentBackBufferIndex(0)
    {
        mIsTearingSupported = CheckTearing();
        mSwapChain = CreateSwapChain(commandQueue);
        UpdateBackBuffers();
    }

    SwapChain::~SwapChain() = default;

    uint32 SwapChain::GetCurrentBackBufferIndex() const
    {
        return mCurrentBackBufferIndex;
    }

    ComPtr<ID3D12Resource> SwapChain::GetCurrentBackBuffer()
    {
        return mBackBuffers[mCurrentBackBufferIndex];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRenderTargetView() const
    {
        return mBackBufferAllocation.GetDescriptor(mCurrentBackBufferIndex);
    }

    uint32 SwapChain::Present()
    {
        bool vSync = true;
        int32 syncInterval = vSync ? 1 : 0;
        int32 presentFlags = mIsTearingSupported && !vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));

        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        return mCurrentBackBufferIndex;
    }

    void SwapChain::Resize(uint32 width, uint32 height)
    {
        mWidth = width;
        mHeight = height;

        for (uint32 i = 0; i < SwapChainBufferCount; ++i)
        {
            mResourceStateTracker->UntrackResource(mBackBuffers[i].Get());
            mBackBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC desc = {};
        ThrowIfFailed(mSwapChain->GetDesc(&desc));
        ThrowIfFailed(mSwapChain->ResizeBuffers(SwapChainBufferCount, width, height, desc.BufferDesc.Format, desc.Flags));

        mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        UpdateBackBuffers();
    }

    ComPtr<IDXGISwapChain4> SwapChain::CreateSwapChain(ID3D12CommandQueue* commandQueue)
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
        sd.OutputWindow = mView.WindowHandle;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | (mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        ThrowIfFailed(mGraphics->GetGIFactory()->CreateSwapChain(commandQueue, &sd, &swapChain));

        ThrowIfFailed(mGraphics->GetGIFactory()->MakeWindowAssociation(mView.WindowHandle, DXGI_MWA_NO_ALT_ENTER));

        ComPtr<IDXGISwapChain4> swapChain4;

        ThrowIfFailed(swapChain.As(&swapChain4));

        return swapChain4;
    }

    void SwapChain::UpdateBackBuffers()
    {
        mBackBufferAllocation = mRTVDescriptorAllocator->Allocate(SwapChainBufferCount);
        for (uint32 i = 0; i < SwapChainBufferCount; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

            auto rtvHandle = mBackBufferAllocation.GetDescriptor(i);
            mGraphics->GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

            backBuffer->SetName((L"BackBuffer" + std::to_wstring(i)).c_str());
            mBackBuffers[i] = backBuffer;

            mResourceStateTracker->TrackResource(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);
        }
    }

    bool SwapChain::CheckTearing()
    {
        BOOL allowTearing = FALSE;

        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(mGraphics->GetGIFactory().As(&factory5)))
        {
            ThrowIfFailed(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));
        }

        return allowTearing == TRUE;
    }

} // namespace Engine