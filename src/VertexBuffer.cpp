#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const std::wstring &name)
    : Buffer(name)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::CreateViews()
{
    mNumVertices = mElementsCount;
    mVertexStride = mElementSize;

    auto size = mNumVertices * mVertexStride;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = mResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = static_cast<uint32>(size);
    vertexBufferView.StrideInBytes = static_cast<uint32>(mVertexStride);

    mVertexBufferView = vertexBufferView;
}