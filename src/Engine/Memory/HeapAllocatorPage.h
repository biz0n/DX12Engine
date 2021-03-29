#pragma once

#include <Types.h>

#include <memory>
#include <d3d12.h>

#include <map>
#include <queue>

namespace Engine::Memory
{
    struct HeapAllocation
    {
        HeapAllocation() : mIsNull{true}
        {
        }

        HeapAllocation(Index offset, Size size) : Offset{offset}, Size{size}, mIsNull{false}
        {
        }

        Index Offset;
        Size Size;

        private:
            bool mIsNull;
    };

    class HeapAllocatorPage : public std::enable_shared_from_this<HeapAllocatorPage>
    {
    public:
        HeapAllocatorPage(uint32 sizePerPage);
        ~HeapAllocatorPage();

        HeapAllocation Allocate(uint32 count = 1);

        void Return(HeapAllocation &&allocation);

    private:
        void AddNewBlock(Index offset, Size size);
        void FreeBlock(Index offset, Size size);

    private:
        uint32 mSizePerPage;

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