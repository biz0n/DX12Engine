#include "IndexPool.h"

namespace Engine::Memory
{
    IndexPool::IndexPool(Size growSize) : mGrowSize {growSize}, mLastIndex {0}
    {
    }

    IndexPool::~IndexPool() = default;


    Index IndexPool::GetIndex()
    {
        if (mFreeIndexes.empty())
        {
            for (Size i = 0; i < mGrowSize; ++i)
            {
                mFreeIndexes.push_back(i + mLastIndex);
            }
            mLastIndex += mGrowSize;
        }

        auto index = mFreeIndexes.front();
        mFreeIndexes.pop_front();
        return index;
    }

    void IndexPool::ReturnIndex(Index index)
    {
        mFreeIndexes.push_back(index);
    }
}