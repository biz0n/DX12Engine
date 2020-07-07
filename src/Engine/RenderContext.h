#pragma once

#include <Types.h>
#include <ResourceStateTracker.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocator.h>
#include <Graphics.h>

#include <d3d12.h>

namespace Engine
{
    class RenderContext
    {
        public:
            RenderContext();

            ~RenderContext() = default;

            ComPtr<ID3D12Device2> Device() const;

            ComPtr<IDXGIFactory4> GIFactory() const;

            DescriptorAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 count = 1);

            SharedPtr<GlobalResourceStateTracker> GetResourceStateTracker() const;

            ComPtr<ID3D12CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

            DescriptorAllocator* GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) {return mDescriptorAllocators[type].get();}

        private:
            ComPtr<ID3D12CommandQueue> CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);

        private:

            SharedPtr<GlobalResourceStateTracker> mGlobalResourceStateTracker;

            UniquePtr<DescriptorAllocator> mDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

            UniquePtr<Graphics> mGraphics;

            ComPtr<ID3D12CommandQueue> mDirrectCommandQueue;
            ComPtr<ID3D12CommandQueue> mComputeCommandQueue;
            ComPtr<ID3D12CommandQueue> mCopyCommandQueue;
    };
}