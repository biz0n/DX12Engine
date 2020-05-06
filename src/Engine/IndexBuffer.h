#pragma once

#include <Types.h>
#include <Buffer.h>

namespace Engine
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const std::wstring &name = L"");
        virtual ~IndexBuffer();

        void CreateViews() override;

        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return mIndexBufferView; }

    private:
        Size mIndecesCount;
        DXGI_FORMAT mIndexFormat;

        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    };

} // namespace Engine