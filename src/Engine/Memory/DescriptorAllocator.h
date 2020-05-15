#pragma once

#include <Types.h>
#include <Exceptions.h>

#include <Memory/DescriptorAllocatorPage.h>
#include <Memory/DescriptorAllocation.h>

#include <cassert>
#include <d3d12.h>
#include <vector>

namespace Engine
{
    class DescriptorAllocator
    {
    public:
        DescriptorAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPerPage = 256);

        DescriptorAllocation Allocate(uint32 count = 1);

        void ReleaseStaleDescriptors(uint64 frameNumber);

    private:
        SharedPtr<DescriptorAllocatorPage> mPage;
    };

} // namespace Engine