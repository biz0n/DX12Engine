#pragma once

#include <Types.h>
#include <Memory/Buffer.h>

namespace Engine::Memory
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const std::wstring &name = L"");
        virtual ~IndexBuffer();

        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

    private:
        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    };

} // namespace Engine::Memory