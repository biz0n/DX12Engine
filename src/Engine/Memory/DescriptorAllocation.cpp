#include "DescriptorAllocation.h"

#include <Exceptions.h>
#include <Memory/DescriptorAllocatorPage.h>

namespace Engine
{
    DescriptorAllocation::DescriptorAllocation()
        : mDescriptor{0}, mNumDescriptors(0), mIncrementalDescriptorSize(0), mPage(nullptr)
    {
    }

    DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32 numDescriptors, uint32 incrementalDescriptorSize, SharedPtr<DescriptorAllocatorPage> page)
        : mDescriptor(descriptor), mNumDescriptors(numDescriptors), mIncrementalDescriptorSize(incrementalDescriptorSize), mPage(page)
    {
    }

    DescriptorAllocation::DescriptorAllocation(DescriptorAllocation &&allocation)
        : mDescriptor(allocation.mDescriptor), mNumDescriptors(allocation.mNumDescriptors), mIncrementalDescriptorSize(allocation.mIncrementalDescriptorSize), mPage(std::move(allocation.mPage))
    {
        allocation.mDescriptor.ptr = 0;
        allocation.mNumDescriptors = 0;
        allocation.mIncrementalDescriptorSize = 0;
    }

    DescriptorAllocation &DescriptorAllocation::operator=(DescriptorAllocation &&other)
    {
        Free();

        mDescriptor = other.mDescriptor;
        mNumDescriptors = other.mNumDescriptors;
        mIncrementalDescriptorSize = other.mIncrementalDescriptorSize;
        mPage = std::move(other.mPage);

        other.mDescriptor.ptr = 0;
        other.mNumDescriptors = 0;
        other.mIncrementalDescriptorSize = 0;

        return *this;
    }

    DescriptorAllocation::~DescriptorAllocation()
    {
        Free();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptor(uint32 offset) const
    {
        assert(offset < mNumDescriptors);
        return {mDescriptor.ptr + (offset * mIncrementalDescriptorSize)};
    }

    uint32 DescriptorAllocation::GetNumDescsriptors() const
    {
        return mNumDescriptors;
    }

    bool DescriptorAllocation::IsNull() const
    {
        return mDescriptor.ptr == 0;
    }

    void DescriptorAllocation::Free()
    {
        if ( !IsNull() && mPage )
        {
            mPage->Free( std::move( *this ) );
            
            mDescriptor.ptr = 0;
            mNumDescriptors = 0;
            mIncrementalDescriptorSize = 0;
            mPage.reset();
        }
    }
} // namespace Engine