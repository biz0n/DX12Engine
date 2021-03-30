#pragma once

#include <Types.h>

#include <Memory/DescriptorAllocator.h>
#include <Memory/IndexPool.h>
#include <HAL/DescriptorHeap.h>

#include <memory>
#include <queue>
#include <functional>
#include <d3d12.h>

namespace Engine::Memory
{
    class DescriptorAllocation
    {
        public:
            using FreeCallback = std::function<void(DescriptorAllocation &&)>;

            DescriptorAllocation();

            DescriptorAllocation(const Descriptor& descriptor, FreeCallback freeCallback);

            // Copies are not allowed.
            DescriptorAllocation(const DescriptorAllocation &) = delete;
            DescriptorAllocation &operator=(const DescriptorAllocation &) = delete;

            DescriptorAllocation(DescriptorAllocation &&allocation);

            DescriptorAllocation &operator=(DescriptorAllocation &&other);

            ~DescriptorAllocation();

            D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor() const { return mCPUDescriptor; }
            D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const { return mGPUDescriptor; }
            Index GetIndex() const { return mIndexInHeap; }

            bool IsNull() const;

            void Free();

        private:
            D3D12_CPU_DESCRIPTOR_HANDLE mCPUDescriptor;
            D3D12_GPU_DESCRIPTOR_HANDLE mGPUDescriptor;
            Index mIndexInHeap;
            FreeCallback mFreeCallback;
    };

    class DescriptorAllocatorPool : public std::enable_shared_from_this<DescriptorAllocatorPool>
    {
        public:
            DescriptorAllocatorPool(ID3D12Device* device, const DescriptorAllocatorConfig& config);
            ~DescriptorAllocatorPool();

        public:
            DescriptorAllocation AllocateRTDescriptor(ID3D12Resource* resource, uint32 mipSlice = 0);
            DescriptorAllocation AllocateDSDescriptor(ID3D12Resource* resource, uint32 mipSlice = 0);
            DescriptorAllocation AllocateSRDescriptor(ID3D12Resource* resource, uint32 strideSize);
            DescriptorAllocation AllocateSRDescriptor(uint32 strideSize);
            DescriptorAllocation AllocateSRCubeDescriptor(ID3D12Resource* resource);
            DescriptorAllocation AllocateSRRaytracingASDescriptor(ID3D12Resource* resource);
            DescriptorAllocation AllocateCBDescriptor(ID3D12Resource* resource, uint32 strideSize);
            DescriptorAllocation AllocateUADescriptor(ID3D12Resource* resource, uint32 strideSize, uint32 mipSlice = 0);
            DescriptorAllocation AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc);
        
        public:
            void SetCurrentFrame(uint64 frameNumber) { mCurrentFrameNumber = frameNumber; };

            void ReleaseStaleDescriptors(uint64 frameNumber);

            D3D12_GPU_DESCRIPTOR_HANDLE GetSRDescriptorHandle() const { return mAllocator.GetSrvCbvUavHeap().GetRange(DescriptorAllocatorConfig::SRVRange).GpuAddress; }
            D3D12_GPU_DESCRIPTOR_HANDLE GetUADescriptorHandle() const { return mAllocator.GetSrvCbvUavHeap().GetRange(DescriptorAllocatorConfig::UAVRange).GpuAddress; }
            D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerDescriptorHandle() const { return mAllocator.GetSamplerHeap().GetRange().GpuAddress; }

            ID3D12DescriptorHeap* GetCbvSrvUavDescriptorHeap() const { return mAllocator.GetSrvCbvUavHeap().D3DHeap(); }
            ID3D12DescriptorHeap* GetSamplerDescriptorHeap() const { return mAllocator.GetSamplerHeap().D3DHeap(); }

        private:
            DescriptorAllocator mAllocator;

            IndexPool mRTIndexPool;
            IndexPool mDSIndexPool;
            IndexPool mCBIndexPool;
            IndexPool mSRIndexPool;
            IndexPool mUAIndexPool;
            IndexPool mSamplerIndexPool;

            uint64 mCurrentFrameNumber;

        private:
            struct StaleDescriptorInfo
            {
                StaleDescriptorInfo(Index index, IndexPool* indexPool, uint64 frameNumber)
                    : Index(index), IndexPool(indexPool), FrameNumber(frameNumber)
                {
                }

                Index Index;
                IndexPool* IndexPool;
                uint64 FrameNumber;
            };

            using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;
            StaleDescriptorQueue mStaleDescriptors;
    };
}