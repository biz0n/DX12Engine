#pragma once

#include <Types.h>
#include <d3d12.h>
#include <cassert>

namespace Engine
{
    class DescriptorAllocatorPage;

    class DescriptorAllocation
    {
    public:
        DescriptorAllocation()
            : mDescriptor{0}, mNumDescriptors(0), mIncrementalDescriptorSize(0), mPage(nullptr)
        {
        }

        DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32 numDescriptors, uint32 incrementalDescriptorSize, SharedPtr<DescriptorAllocatorPage> page)
            : mDescriptor(descriptor)
            , mNumDescriptors(numDescriptors)
            , mIncrementalDescriptorSize(incrementalDescriptorSize)
            , mPage(page)
        {
        }

        // Copies are not allowed.
        DescriptorAllocation( const DescriptorAllocation& ) = delete;
        DescriptorAllocation& operator=( const DescriptorAllocation& ) = delete;

        DescriptorAllocation( DescriptorAllocation&& allocation )
            : mDescriptor(allocation.mDescriptor)
            , mNumDescriptors(allocation.mNumDescriptors)
            , mIncrementalDescriptorSize(allocation.mIncrementalDescriptorSize)
            , mPage(std::move(allocation.mPage))
        {
            allocation.mDescriptor.ptr = 0;
            allocation.mNumDescriptors = 0;
            allocation.mIncrementalDescriptorSize = 0;
        }

        DescriptorAllocation& operator=( DescriptorAllocation&& other )
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

        ~DescriptorAllocation()
        {
            Free();
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor(uint32 offset = 0) const
        {
            assert(offset < mNumDescriptors);
            return {mDescriptor.ptr + (offset * mIncrementalDescriptorSize)};
        }

        uint32 GetNumDescsriptors() const { return mNumDescriptors; }

        bool IsNull() const
        {
            return mDescriptor.ptr == 0;
        }

        void Free()
        {
            if (!IsNull())
            {

            }

            if (mPage)
            {
                mPage.reset();
            }
        }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE mDescriptor;
        uint32 mNumDescriptors;
        uint32 mIncrementalDescriptorSize;
        SharedPtr<DescriptorAllocatorPage> mPage;
    };
}