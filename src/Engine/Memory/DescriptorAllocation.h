#pragma once

#include <Types.h>

#include <d3d12.h>

namespace Engine
{
    class DescriptorAllocatorPage;

    class DescriptorAllocation
    {
    public:
        DescriptorAllocation();

        DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32 numDescriptors, uint32 incrementalDescriptorSize, SharedPtr<DescriptorAllocatorPage> page);

        // Copies are not allowed.
        DescriptorAllocation(const DescriptorAllocation &) = delete;
        DescriptorAllocation &operator=(const DescriptorAllocation &) = delete;

        DescriptorAllocation(DescriptorAllocation &&allocation);

        DescriptorAllocation &operator=(DescriptorAllocation &&other);

        ~DescriptorAllocation();

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor(uint32 offset = 0) const;

        uint32 GetNumDescsriptors() const;

        bool IsNull() const;

        void Free();

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE mDescriptor;
        uint32 mNumDescriptors;
        uint32 mIncrementalDescriptorSize;
        SharedPtr<DescriptorAllocatorPage> mPage;
    };
} // namespace Engine