#pragma once

#include <Types.h>

#include <memory>
#include <d3d12.h>

#include <map>
#include <queue>

namespace Engine::Memory
{
    class DescriptorAllocation;

    class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
    {
    public:
        DescriptorAllocatorPage(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPerPage = 256);
        ~DescriptorAllocatorPage() = default;

        DescriptorAllocation Allocate(uint32 count = 1);

        void SetCurrentFrame(uint64 frameNumber) { mCurrentFrameNumber = frameNumber; };
        void Free(DescriptorAllocation &&descriptorHandle);

        void ReleaseStaleDescriptors(uint64 frameNumber);

    private:
        void AddNewBlock(Index offset, Size size);
        void FreeBlock(Index offset, Size size);
        Index CalculateOffset(const D3D12_CPU_DESCRIPTOR_HANDLE &handle);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        uint32 mNumDescriptorsPerPage;
        ComPtr<ID3D12DescriptorHeap> mHeap;
        uint32 mDescriptorHandleIncrementSize;
        D3D12_CPU_DESCRIPTOR_HANDLE mBaseDescriptor;

        uint64 mCurrentFrameNumber;

    private:
        struct StaleDescriptorInfo
        {
            StaleDescriptorInfo(Index offset, Size count, uint64 frameNumber)
                : Offset(offset), Count(count), FrameNumber(frameNumber)
            {
            }

            Index Offset;
            Size Count;
            uint64 FrameNumber;
        };

        using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;
        StaleDescriptorQueue mStaleDescriptors;

    private:
        struct FreeBlockInfo;

        using FreeBlockByOffsetMap = std::map<Index, FreeBlockInfo>;

        using FreeBlockBySizeMap = std::multimap<Size, FreeBlockByOffsetMap::iterator>;

        struct FreeBlockInfo
        {
            Size BlockSize;

            FreeBlockBySizeMap::iterator OrderBySizeIt;

            FreeBlockInfo(Size size) : BlockSize(size)
            {
            }
        };

        FreeBlockByOffsetMap mFreeBlockByOffsetMap;
        FreeBlockBySizeMap mFreeBlockBySizeMap;
        Size mFreeSize;
    };
} // namespace Engine::Memory