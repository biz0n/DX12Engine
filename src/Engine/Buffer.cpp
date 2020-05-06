#include "Buffer.h"

Buffer::Buffer(const std::wstring &name)
    : Resource(name)
{
}

Buffer::~Buffer()
{
    mBuffer.clear();
}

void Buffer::SetData(Size elementsCount, Size elementSize, const std::vector<Byte> &data)
{
    mElementsCount = elementsCount;
    mElementSize = elementSize;

    mBuffer = std::move(data);
}