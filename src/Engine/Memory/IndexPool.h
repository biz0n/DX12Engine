#pragma once

#include <Types.h>

#include <list>

namespace Engine::Memory
{
    class IndexPool
    {
        public:
            IndexPool(Size growSize);
            ~IndexPool();

            // Copies are not allowed.
            IndexPool(const IndexPool &) = delete;
            IndexPool &operator=(const IndexPool &) = delete;

            Index GetIndex();
            void ReturnIndex(Index index);

        private:
            std::list<Index> mFreeIndexes;
            Size mGrowSize;
            Index mLastIndex;
    };
}