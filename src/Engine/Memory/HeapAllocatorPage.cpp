#include "HeapAllocatorPage.h"

namespace Engine::Memory
{
    HeapAllocatorPage::HeapAllocatorPage(uint32 sizePerPage)
        : mFreeSize(sizePerPage)
    {
        AddNewBlock(0, sizePerPage);
    }

    HeapAllocatorPage::~HeapAllocatorPage() = default;

    HeapAllocation HeapAllocatorPage::Allocate(uint32 count)
    {
        if (mFreeSize < count)
        {
            return HeapAllocation();
        }

        auto smallestBlockIt = mFreeBlockBySizeMap.lower_bound(count);
        if (smallestBlockIt == mFreeBlockBySizeMap.end())
        {
            return HeapAllocation();
        }
        auto smallestBlock = smallestBlockIt->second;
        auto offset = smallestBlock->first;

        auto newOffset = offset + count;
        auto newSize = smallestBlock->second.BlockSize - count;

        mFreeBlockBySizeMap.erase(smallestBlockIt);
        mFreeBlockByOffsetMap.erase(smallestBlock);

        if (newSize > 0)
        {
            AddNewBlock(newOffset, newSize);
        }

        mFreeSize -= count;

        return HeapAllocation(offset, count);
    }

    void HeapAllocatorPage::Return(HeapAllocation &&allocation)
    {
        auto offset = allocation.Offset;
        auto count = allocation.Size;

        FreeBlock(offset, count);
    }

    void HeapAllocatorPage::AddNewBlock(Index offset, Size size)
    {
        auto newBlockIt = mFreeBlockByOffsetMap.emplace(offset, size);
        auto orderIt = mFreeBlockBySizeMap.emplace(size, newBlockIt.first);
        newBlockIt.first->second.OrderBySizeIt = orderIt;
    }

    void HeapAllocatorPage::FreeBlock(Index offset, Size size)
    {
        auto nextBlockIt = mFreeBlockByOffsetMap.upper_bound(offset);
        auto prevBlockIt = nextBlockIt;

        if (prevBlockIt != mFreeBlockByOffsetMap.begin())
        {
            prevBlockIt--;
        }
        else
        {
            prevBlockIt = mFreeBlockByOffsetMap.end();
        }

        if (prevBlockIt != mFreeBlockByOffsetMap.end() && offset == prevBlockIt->first + prevBlockIt->second.BlockSize)
        {
            // PrevBlock.Offset           Offset
            // |                          |
            // |<-----PrevBlock.Size----->|<------Size-------->|
            //
            size += prevBlockIt->second.BlockSize;
            offset = prevBlockIt->first;

            mFreeBlockBySizeMap.erase(prevBlockIt->second.OrderBySizeIt);
            mFreeBlockByOffsetMap.erase(prevBlockIt);
        }
        if (nextBlockIt != mFreeBlockByOffsetMap.end() && offset + size == nextBlockIt->first)
        {
            // Offset               NextBlock.Offset
            // |                    |
            // |<------Size-------->|<-----NextBlock.Size----->|
            //
            size += nextBlockIt->second.BlockSize;
            mFreeBlockBySizeMap.erase(nextBlockIt->second.OrderBySizeIt);
            mFreeBlockByOffsetMap.erase(nextBlockIt);
        }

        AddNewBlock(offset, size);

        mFreeSize += size;
    }

} // namespace Engine::Memory