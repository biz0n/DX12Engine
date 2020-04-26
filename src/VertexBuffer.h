#pragma once

#include "Types.h"
#include "Buffer.h"
#include <d3d12.h>

class VertexBuffer : public Buffer
{
public:
    VertexBuffer(const std::wstring &name = L"");
    virtual ~VertexBuffer();
    void CreateViews() override;

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return mVertexBufferView; }

private:
    Size mNumVertices;
    Size mVertexStride;

    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
};