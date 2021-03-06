#pragma once

#include <Types.h>

#include <View.h>
#include <EngineConfig.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Memory/MemoryForwards.h>

#include <Render/RenderForwards.h>

namespace Engine
{
    struct View;
}

namespace Engine::Render
{
    class SwapChain
    {
    public:
        SwapChain(View view, const Graphics* graphis, SharedPtr<GlobalResourceStateTracker> resourceStateTracker, ID3D12CommandQueue* commandQueue);
        ~SwapChain();

        uint32 GetCurrentBackBufferIndex() const;
        Texture* GetCurrentBackBufferTexture();

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
        UniquePtr<Texture> mBackBufferTextures[EngineConfig::SwapChainBufferCount];

        SharedPtr<GlobalResourceStateTracker> mResourceStateTracker;

        uint32 mCurrentBackBufferIndex;

        const Graphics* mGraphics;

        uint32 mWidth;
        uint32 mHeight;
        View mView;
    };
} // namespace Engine::Render