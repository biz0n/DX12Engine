#include "DescriptorAllocatorPool.h"

namespace Engine::Memory
{
    DescriptorAllocation::DescriptorAllocation()
    : mCPUDescriptor{0}, mGPUDescriptor{0}, mIndexInHeap{0}, mFreeCallback{0}
    {
    }

    DescriptorAllocation::DescriptorAllocation(const Descriptor& descriptor, FreeCallback freeCallback)
        : mCPUDescriptor{descriptor.CpuAddress}, mGPUDescriptor{descriptor.GpuAddress}, mIndexInHeap{descriptor.HeapIndex}, mFreeCallback{freeCallback}
    {
    }

    DescriptorAllocation::DescriptorAllocation(DescriptorAllocation &&allocation)
        : mCPUDescriptor{allocation.mCPUDescriptor}, mGPUDescriptor{allocation.mGPUDescriptor}, mIndexInHeap{allocation.mIndexInHeap}, mFreeCallback{allocation.mFreeCallback}
    {
        allocation.mCPUDescriptor.ptr = 0;
        allocation.mGPUDescriptor.ptr = 0;
        allocation.mIndexInHeap = 0;
        allocation.mFreeCallback = nullptr;
    }

    DescriptorAllocation &DescriptorAllocation::operator=(DescriptorAllocation &&other)
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

    DescriptorAllocation::~DescriptorAllocation()
    {
        Free();
    }


    bool DescriptorAllocation::IsNull() const
    {
        return mCPUDescriptor.ptr == 0;
    }

    void DescriptorAllocation::Free()
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

    DescriptorAllocation DescriptorAllocatorPool::AllocateRTDescriptor(ID3D12Resource* resource, uint32 mipSlice)
    {
        const auto index = mRTIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateRTDescriptor(resource, index, mipSlice);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mRTIndexPool, mCurrentFrameNumber);
        });
    }
    
    DescriptorAllocation DescriptorAllocatorPool::AllocateDSDescriptor(ID3D12Resource* resource, uint32 mipSlice)
    {
        const auto index = mDSIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateDSDescriptor(resource, index, mipSlice);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mDSIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateSRDescriptor(ID3D12Resource* resource, uint32 strideSize)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRDescriptor(resource, index, strideSize);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateSRDescriptor(uint32 strideSize)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRDescriptor(index, strideSize);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateSRCubeDescriptor(ID3D12Resource* resource)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRCubeDescriptor(resource, index);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateSRRaytracingASDescriptor(ID3D12Resource* resource)
    {
        const auto index = mSRIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSRRaytracingASDescriptor(resource, index);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mSRIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateCBDescriptor(ID3D12Resource* resource, uint32 strideSize)
    {
        const auto index = mCBIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateCBDescriptor(resource, index, strideSize);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mCBIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateUADescriptor(ID3D12Resource* resource, uint32 strideSize, uint32 mipSlice)
    {
        const auto index = mUAIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateUADescriptor(resource, index, strideSize, mipSlice);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
        {
            mStaleDescriptors.emplace(allocation.GetIndex(), &mUAIndexPool, mCurrentFrameNumber);
        });
    }

    DescriptorAllocation DescriptorAllocatorPool::AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc)
    {
        const auto index = mSamplerIndexPool.GetIndex();
        const auto descriptor = mAllocator.AllocateSamplerDescriptor(samplerDesc, index);
        return DescriptorAllocation(descriptor, [this](DescriptorAllocation&& allocation)
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