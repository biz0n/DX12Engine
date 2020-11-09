#pragma once

#include <View.h>
#include <Types.h>
#include <EventTracker.h>
#include <Memory/MemoryForwards.h>
#include <Render/SwapChain.h>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace Engine
{
    namespace Render
    {
        class PipelineStateProvider;
        class ShaderProvider;
    }

    class SwapChain;
    class CommandAllocatorPool;
    class UIRenderContext;
    class Graphics;
    class CommandQueue;
    class ResourceStateTracker;
    class GlobalResourceStateTracker;

    class RenderContext
    {
        public:
            RenderContext(View view);

            ~RenderContext();

            ComPtr<ID3D12Device2> Device() const;

            ComPtr<IDXGIFactory4> GIFactory() const;

            SharedPtr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;

            SharedPtr<DescriptorAllocator> GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) const { return mDescriptorAllocators[type]; }

            ComPtr<ID3D12GraphicsCommandList> CreateCommandList(D3D12_COMMAND_LIST_TYPE type);

            SharedPtr<SwapChain> GetSwapChain() const { return mSwapChain; }

            SharedPtr<GlobalResourceStateTracker> GetGlobalResourceStateTracker() const { return mGlobalResourceStateTracker ;}

            SharedPtr<UIRenderContext> GetUIContext() const { return mUIRenderContext; }

            Render::PipelineStateProvider* GetPripelineStateProvider() { return mPipelineStateProvider.get(); }

            Render::ShaderProvider* GetShaderProvider() { return mShaderProvider.get(); }

            EventTracker& GetEventTracker() {return mEventTracker; }

            uint64 GetFrameCount() const { return mFrameCount; }

            void BeginFrame();

            void EndFrame();

        public:
            SharedPtr<CommandQueue> GetGraphicsCommandQueue() const { return GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT); };
            SharedPtr<CommandQueue> GetCopyCommandQueue() const { return GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY); };
            SharedPtr<CommandQueue> GetComputeCommandQueue() const { return GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE); };

            ComPtr<ID3D12GraphicsCommandList> CreateGraphicsCommandList() { return CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT); }
            ComPtr<ID3D12GraphicsCommandList> CreateCopyCommandList() { return CreateCommandList(D3D12_COMMAND_LIST_TYPE_COPY); }
            ComPtr<ID3D12GraphicsCommandList> CreateComputeCommandList() { return CreateCommandList(D3D12_COMMAND_LIST_TYPE_COMPUTE); }

            uint32 GetCurrentBackBufferIndex() const { return mSwapChain->GetCurrentBackBufferIndex(); }

            void WaitForIdle();

        private:

            SharedPtr<GlobalResourceStateTracker> mGlobalResourceStateTracker;

            SharedPtr<DescriptorAllocator> mDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

            UniquePtr<Graphics> mGraphics;

            SharedPtr<CommandQueue> mDirrectCommandQueue;
            SharedPtr<CommandQueue> mComputeCommandQueue;
            SharedPtr<CommandQueue> mCopyCommandQueue;

            UniquePtr<CommandAllocatorPool> mCommandAllocators[EngineConfig::SwapChainBufferCount];

            SharedPtr<SwapChain> mSwapChain;

            SharedPtr<UIRenderContext> mUIRenderContext;

            UniquePtr<Render::PipelineStateProvider> mPipelineStateProvider;
            UniquePtr<Render::ShaderProvider> mShaderProvider;

            EventTracker mEventTracker;

            uint64 mFrameCount;
            uint64 mFenceValues[EngineConfig::SwapChainBufferCount] = {0};
            uint64 mFrameValues[EngineConfig::SwapChainBufferCount] = {0};
    };
}