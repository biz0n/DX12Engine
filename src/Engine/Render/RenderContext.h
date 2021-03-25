#pragma once

#include <View.h>
#include <Types.h>
#include <EventTracker.h>
#include <EngineConfig.h>
#include <Memory/MemoryForwards.h>
#include <Render/RenderForwards.h>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace Engine::Render
{
    class RenderContext
    {
    public:
        RenderContext(View view);

        ~RenderContext();

        ComPtr<ID3D12Device2> Device() const;

        ComPtr<IDXGIFactory4> GIFactory() const;

        SharedPtr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;

        SharedPtr<Memory::DescriptorAllocator> GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) const { return mDescriptorAllocators[type]; }

        ComPtr<ID3D12GraphicsCommandList> CreateCommandList(D3D12_COMMAND_LIST_TYPE type);

        SharedPtr<SwapChain> GetSwapChain() const { return mSwapChain; }

        SharedPtr<GlobalResourceStateTracker> GetGlobalResourceStateTracker() const { return mGlobalResourceStateTracker; }

        SharedPtr<UIRenderContext> GetUIContext() const { return mUIRenderContext; }

        Render::PipelineStateProvider *GetPipelineStateProvider() { return mPipelineStateProvider.get(); }

        Render::ShaderProvider *GetShaderProvider() { return mShaderProvider.get(); }
        Render::RootSignatureProvider *GetRootSignatureProvider() { return mRootSignatureProvider.get(); }

        Memory::ResourceFactory* GetResourceFactory() { return mResourceFactory.get(); }
        Memory::ResourceCopyManager* GetResourceCopyManager() { return mResourceCopyManager.get(); }

        EventTracker &GetEventTracker() { return mEventTracker; }

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

        uint32 GetCurrentBackBufferIndex() const;

        void WaitForIdle();

    private:
        SharedPtr<GlobalResourceStateTracker> mGlobalResourceStateTracker;

        SharedPtr<Memory::DescriptorAllocator> mDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        SharedPtr<Memory::DescriptorAllocatorPool> mDescriptorAllocatorPool;

        UniquePtr<Graphics> mGraphics;

        SharedPtr<CommandQueue> mDirectCommandQueue;
        SharedPtr<CommandQueue> mComputeCommandQueue;
        SharedPtr<CommandQueue> mCopyCommandQueue;

        UniquePtr<Memory::CommandAllocatorPool> mCommandAllocators[EngineConfig::SwapChainBufferCount];

        SharedPtr<SwapChain> mSwapChain;

        SharedPtr<UIRenderContext> mUIRenderContext;

        UniquePtr<Render::PipelineStateProvider> mPipelineStateProvider;
        UniquePtr<Render::ShaderProvider> mShaderProvider;
        UniquePtr<Render::RootSignatureProvider> mRootSignatureProvider;

        UniquePtr<Memory::ResourceAllocator> mResourceAllocator;
        UniquePtr<Memory::ResourceFactory> mResourceFactory;
        UniquePtr<Memory::ResourceCopyManager> mResourceCopyManager;


        EventTracker mEventTracker;

        uint64 mFrameCount;
        uint64 mFenceValues[EngineConfig::SwapChainBufferCount] = {0};
        uint64 mFrameValues[EngineConfig::SwapChainBufferCount] = {0};
    };
} // namespace Engine::Render