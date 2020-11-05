#include "VertexBuffer.h"

namespace Engine
{
    VertexBuffer::VertexBuffer(const std::wstring &name)
        : Buffer(name), mVertexBufferView{0}
    {
    }

    VertexBuffer::~VertexBuffer()
    {
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVertexBufferView()
    {
        if (!mVertexBufferView.BufferLocation)
        {
            auto size = mElementsCount * mElementSize;

            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
            vertexBufferView.BufferLocation = mResource->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = static_cast<uint32>(size);
            vertexBufferView.StrideInBytes = static_cast<uint32>(mElementSize);

            mVertexBufferView = vertexBufferView;
        }

        return mVertexBufferView;
    }

} // namespace Engine