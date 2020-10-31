#pragma once

#include <Types.h>
#include <Resource.h>

#include <vector>

namespace Engine
{
    class Buffer : public Resource
    {
    public:
        Buffer(const std::wstring &name);
        virtual ~Buffer();

        template <typename T>
        void SetData(const std::vector<T> &data)
        {
            mElementsCount = data.size();
            mElementSize = sizeof(T);
            Size size = mElementSize * mElementsCount;

            mBuffer.reserve(size);

            memcpy(mBuffer.data(), data.data(), size);
        }

        void SetData(Size elementsCount, Size elementSize, const std::vector<Byte> &data);

        Size GetElementsCount() const { return mElementsCount; }
        Size GetElementSize() const { return mElementSize; }

        const void *GetData() const { return mBuffer.data(); }

    protected:
        Size mElementsCount;
        Size mElementSize;

    private:
        std::vector<Byte> mBuffer;
    };

} // namespace Engine