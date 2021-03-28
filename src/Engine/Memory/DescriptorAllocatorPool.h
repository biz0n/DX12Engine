#pragma once

#include <Types.h>

#include <Memory/NewDescriptorAllocator.h>
#include <Memory/IndexPool.h>

#include <memory>
#include <queue>
#include <functional>
#include <d3d12.h>

namespace Engine::Memory
{
    class NewDescriptorAllocation
    {
        public:
            using FreeCallback = std::function<void(NewDescriptorAllocation &&)>;

            NewDescriptorAllocation();

            NewDescriptorAllocation(const Descriptor& descriptor, FreeCallback freeCallback);

            // Copies are not allowed.
            NewDescriptorAllocation(const NewDescriptorAllocation &) = delete;
            NewDescriptorAllocation &operator=(const NewDescriptorAllocation &) = delete;

            NewDescriptorAllocation(NewDescriptorAllocation &&allocation);

            NewDescriptorAllocation &operator=(NewDescriptorAllocation &&other);

            ~NewDescriptorAllocation();

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
            NewDescriptorAllocation AllocateRTDescriptor(ID3D12Resource* resource, uint32 mipSlice = 0);
            NewDescriptorAllocation AllocateDSDescriptor(ID3D12Resource* resource, uint32 mipSlice = 0);
            NewDescriptorAllocation AllocateSRDescriptor(ID3D12Resource* resource, uint32 strideSize);
            NewDescriptorAllocation AllocateSRDescriptor(uint32 strideSize);
            NewDescriptorAllocation AllocateSRCubeDescriptor(ID3D12Resource* resource);
            NewDescriptorAllocation AllocateSRRaytracingASDescriptor(ID3D12Resource* resource);
            NewDescriptorAllocation AllocateCBDescriptor(ID3D12Resource* resource, uint32 strideSize);
            NewDescriptorAllocation AllocateUADescriptor(ID3D12Resource* resource, uint32 strideSize, uint32 mipSlice = 0);
            NewDescriptorAllocation AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc);
        
        public:
            void SetCurrentFrame(uint64 frameNumber) { mCurrentFrameNumber = frameNumber; };

            void ReleaseStaleDescriptors(uint64 frameNumber);

            D3D12_GPU_DESCRIPTOR_HANDLE GetSRDescriptorHandle() const { return mAllocator.GetSrvCbvUavHeap().GetRange(DescriptorAllocatorConfig::SRVRange).GpuAddress; }
            D3D12_GPU_DESCRIPTOR_HANDLE GetUADescriptorHandle() const { return mAllocator.GetSrvCbvUavHeap().GetRange(DescriptorAllocatorConfig::UAVRange).GpuAddress; }
            D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerDescriptorHandle() const { return mAllocator.GetSamplerHeap().GetRange().GpuAddress; }

            ID3D12DescriptorHeap* GetCbvSrvUavDescriptorHeap() const { return mAllocator.GetSrvCbvUavHeap().D3DHeap(); }
            ID3D12DescriptorHeap* GetSamplerDescriptorHeap() const { return mAllocator.GetSamplerHeap().D3DHeap(); }

        private:
            NewDescriptorAllocator mAllocator;

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