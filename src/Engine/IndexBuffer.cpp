#include "IndexBuffer.h"

#include <cassert>

namespace Engine
{
    IndexBuffer::IndexBuffer(const std::wstring &name)
        : Buffer(name)
    {
    }

    IndexBuffer::~IndexBuffer()
    {
    }

    void IndexBuffer::CreateViews()
    {
        assert(mElementSize == 2 || mElementSize == 4 && "Indices must be 16, or 32-bit integers.");
        mIndecesCount = mElementsCount;
        mIndexFormat = mElementSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

        auto size = mElementsCount * mElementSize;

        D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        indexBufferView.BufferLocation = mResource->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes = static_cast<uint32>(size);
        indexBufferView.Format = mIndexFormat;

        mIndexBufferView = indexBufferView;
    }

} // namespace Engine