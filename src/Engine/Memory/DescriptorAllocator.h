#pragma once

#include <Types.h>
#include <Exceptions.h>

#include <Memory/DescriptorAllocatorPool.h>

#include <cassert>
#include <d3d12.h>
#include <vector>

namespace Engine::Memory
{
    class DescriptorAllocatorPage;
    class DescriptorAllocation;

    class DescriptorAllocator
    {
    public:
        DescriptorAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, SharedPtr<DescriptorAllocatorPool> newPool, uint32 numDescriptorsPerPage = 256);
        ~DescriptorAllocator() = default;

        DescriptorAllocation Allocate(uint32 count = 1);

        void ReleaseStaleDescriptors(uint64 frameNumber);

        SharedPtr<DescriptorAllocatorPool> NewPool;

    private:
        SharedPtr<DescriptorAllocatorPage> mPage;
    };

} // namespace Engine::Memory