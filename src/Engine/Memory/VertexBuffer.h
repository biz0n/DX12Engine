#pragma once

#include <Types.h>
#include <Memory/Buffer.h>
#include <d3d12.h>

namespace Engine::Memory
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const std::wstring &name = L"");
        virtual ~VertexBuffer();

        D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();

    private:
        D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
    };

} // namespace Engine::Memory