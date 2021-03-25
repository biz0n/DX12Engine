#include "IndexBuffer.h"

#include <cassert>

namespace Engine::Memory
{
    IndexBuffer::IndexBuffer(ID3D12Device *device,
                             ResourceAllocator *resourceFactory,
                             DescriptorAllocatorPool *descriptorAllocator,
                             Engine::Render::GlobalResourceStateTracker* stateTracker,
                             Size elementsCount,
                             Size stride,
                             D3D12_RESOURCE_STATES state)
        : Buffer(device, resourceFactory, descriptorAllocator, stateTracker, stride, CD3DX12_RESOURCE_DESC::Buffer(elementsCount * stride), state, D3D12_HEAP_TYPE_DEFAULT), mIndexBufferView{0}
    {
    }

    IndexBuffer::~IndexBuffer() = default;

    D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetIndexBufferView()
    {
        if (!mIndexBufferView.BufferLocation)
        {
            const auto elementSize = GetStride();
            assert(elementSize == 2 || elementSize == 4 && "Indices must be 16, or 32-bit integers.");
            auto indexFormat = elementSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
            indexBufferView.BufferLocation = D3DResource()->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = static_cast<uint32>(GetDescription().Width);
            indexBufferView.Format = indexFormat;

            mIndexBufferView = indexBufferView;
        }

        return mIndexBufferView;
    }

} // namespace Engine::Memory