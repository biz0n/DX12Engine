#include "DescriptorAllocatorPool.h"

namespace Engine::Memory
{
    NewDescriptorAllocation::NewDescriptorAllocation()
    : mCPUDescriptor{0}, mGPUDescriptor{0}, mIndexInHeap{0}, mFreeCallback{0}
    {
    }

    NewDescriptorAllocation::NewDescriptorAllocation(const Descriptor& descriptor, FreeCallback freeCallback)
        : mCPUDescriptor{descriptor.CpuAddress}, mGPUDescriptor{descriptor.GpuAddress}, mIndexInHeap{descriptor.HeapIndex}, mFreeCallback{freeCallback}
    {
    }

    NewDescriptorAllocation::NewDescriptorAllocation(NewDescriptorAllocation &&allocation)
        : mCPUDescriptor{allocation.mCPUDescriptor}, mGPUDescriptor{allocation.mGPUDescriptor}, mIndexInHeap{allocation.mIndexInHeap}, mFreeCallback{allocation.mFreeCallback}
    {
        allocation.mCPUDescriptor.ptr = 0;
        allocation.mGPUDescriptor.ptr = 0;
        allocation.mIndexInHeap = 0;
        allocation.mFreeCallback = nullptr;
    }

    NewDescriptorAllocation &NewDescriptorAllocation::operator=(NewDescriptorAllocation &&other)
    {
        Free();

        mCPUDescriptor = other.mCPUDescriptor;
        mGPUDescriptor = other.mGPUDescriptor;
        mIndexInHeap = other.mIndexInHeap;
        mFreeCallback = other.mFreeCallback;

        other.mCPUDescriptor.ptr = 0;
        other.mGPUDescriptor.ptr = 0;
        other.mIndexInHeap = 0;
        other.mFreeCallback = nullptr;

        return *this;
    }

    NewDescriptorAllocation::~NewDescriptorAllocation()
    {
        Free();
    }


    bool NewDescriptorAllocation::IsNull() const
    {
        return mCPUDescriptor.ptr == 0;
    }

    void NewDescriptorAllocation::Free()
    {
        if (!IsNull() && mFreeCallback)
        {
            mFreeCallback(std::move(*this));

            mCPUDescriptor.ptr = 0;
            mGPUDescriptor.ptr = 0;
            mIndexInHeap = 0;
            mFreeCallback = nullptr;
        }
    }

    DescriptorAllocatorPool::DescriptorAllocatorPool(ID3D12Device* device, const DescriptorAllocatorConfig& config) :
        mAllocator {device, config},
        mRTIndexPool {config.RenderTargetHeapSize},
        mDSIndexPool {config.DepthStencilHeapSize},
        mCBIndexPool {config.ConstantBufferViewRange},
        mSRIndexPool {config.ShaderResourceViewRange},
        mUAIndexPool {config.UnorderedAccessViewRange},
        mSamplerIndexPool {config.SamplerHeapSize},
        mCurrentFrameNumber {0}
    {

    }

    DescriptorAllocatorPool::~DescriptorAllocatorPool() = default;

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateRTDescriptor(ID3D12Resource* resource, uint32 mipSlice)
    {
        const auto index = mRTIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateRTDescriptor(resource, index, mipSlice);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mRTIndexPool, mCurrentFrameNumber);
        });
    }
    
    NewDescriptorAllocation DescriptorAllocatorPool::AllocateDSDescriptor(ID3D12Resource* resource, uint32 mipSlice)
    {
        const auto index = mDSIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateDSDescriptor(resource, index, mipSlice);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mDSIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateSRDescriptor(ID3D12Resource* resource, uint32 strideSize)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRDescriptor(resource, index, strideSize);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateSRDescriptor(uint32 strideSize)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRDescriptor(index, strideSize);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateSRCubeDescriptor(ID3D12Resource* resource)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRCubeDescriptor(resource, index);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateSRRaytracingASDescriptor(ID3D12Resource* resource)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRRaytracingASDescriptor(resource, index);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateCBDescriptor(ID3D12Resource* resource, uint32 strideSize)
    {
        const auto index = mCBIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateCBDescriptor(resource, index, strideSize);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mCBIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateUADescriptor(ID3D12Resource* resource, uint32 strideSize, uint32 mipSlice)
    {
        const auto index = mUAIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateUADescriptor(resource, index, strideSize, mipSlice);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mUAIndexPool, mCurrentFrameNumber);
        });
    }

    NewDescriptorAllocation DescriptorAllocatorPool::AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc)
    {
        const auto index = mSamplerIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSamplerDescriptor(samplerDesc, index);
        return NewDescriptorAllocation(descriptor, [this](NewDescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSamplerIndexPool, mCurrentFrameNumber);
        });
    }

    void DescriptorAllocatorPool::ReleaseStaleDescriptors(uint64 frameNumber)
    {
        while (!mStaleDescriptors.empty() && mStaleDescriptors.front().FrameNumber <= frameNumber)
        {
            auto &staleDescriptor = mStaleDescriptors.front();

            auto indexPool = staleDescriptor.IndexPool;
            auto index = staleDescriptor.Index;
            indexPool->ReturnIndex(index);

            mStaleDescriptors.pop();
        }
    }
}