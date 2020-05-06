#pragma once

#include <Types.h>
#include <Exceptions.h>

#include <cassert>
#include <d3d12.h>
#include <vector>

namespace Engine
{

    class DescriptorAllocation
    {
    public:
        DescriptorAllocation()
            : mDescriptor{0}, mNumDescriptors(0), mIncrementalDescriptorSize(0)
        {
        }

        DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32 numDescriptors, uint32 incrementalDescriptorSize)
            : mDescriptor(descriptor), mNumDescriptors(numDescriptors), mIncrementalDescriptorSize(incrementalDescriptorSize)
        {
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor(uint32 offset = 0) const
        {
            assert(offset < mNumDescriptors);
            return {mDescriptor.ptr + (offset * mIncrementalDescriptorSize)};
        }

        uint32 GetNumDescsriptors() const { return mNumDescriptors; }

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE mDescriptor;
        uint32 mNumDescriptors;
        uint32 mIncrementalDescriptorSize;
    };

    class DescriptorAllocator
    {
    public:
        DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 incrementalDescriptorSize, uint32 numDescriptorsPerPage = 256)
            : mHeapType(type), mIncrementalDescriptorSize(incrementalDescriptorSize), mNumDescriptorsPerPage(numDescriptorsPerPage)
        {
        }

        DescriptorAllocation Allocate(ComPtr<ID3D12Device> device, uint32 count = 1)
        {
            assert(count <= mNumDescriptorsPerPage);

            if (mCurrentHeap == nullptr || (mCurrentDescriptorOffset + count) > mNumDescriptorsPerPage)
            {
                if (mCurrentHeap != nullptr)
                {
                    mHeapsPool.emplace_back(mCurrentHeap);
                }
                mCurrentHeap = CreateDescriptorHeap(device);
                mCurrentHandle = mCurrentHeap->GetCPUDescriptorHandleForHeapStart();
                mCurrentDescriptorOffset = 0;
            }

            D3D12_CPU_DESCRIPTOR_HANDLE descriptor{mCurrentHandle.ptr + (mCurrentDescriptorOffset * mIncrementalDescriptorSize)};
            DescriptorAllocation allocation(descriptor, count, mIncrementalDescriptorSize);

            mCurrentDescriptorOffset += count;

            return allocation;
        }

    private:
        ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device> device)
        {
            ComPtr<ID3D12DescriptorHeap> heap;
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = mNumDescriptorsPerPage;
            heapDesc.Type = mHeapType;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

            return heap;
        }

        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        std::vector<ComPtr<ID3D12DescriptorHeap>> mHeapsPool;
        uint32 mIncrementalDescriptorSize;
        uint32 mNumDescriptorsPerPage;

        ComPtr<ID3D12DescriptorHeap> mCurrentHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE mCurrentHandle;
        uint32 mCurrentDescriptorOffset;
    };

} // namespace Engine