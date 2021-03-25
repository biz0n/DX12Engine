#include "VertexBuffer.h"

namespace Engine::Memory
{
    VertexBuffer::VertexBuffer(ID3D12Device *device,
                               ResourceAllocator *resourceFactory,
                               DescriptorAllocatorPool *descriptorAllocator,
                               Engine::Render::GlobalResourceStateTracker* stateTracker,
                               Size elementsCount,
                               Size stride,
                               D3D12_RESOURCE_STATES state)
        : Buffer(device, resourceFactory, descriptorAllocator, stateTracker, stride, CD3DX12_RESOURCE_DESC::Buffer(elementsCount * stride), state, D3D12_HEAP_TYPE_DEFAULT), mVertexBufferView{0}
    {
    }

    VertexBuffer::~VertexBuffer() = default;

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVertexBufferView()
    {
        if (!mVertexBufferView.BufferLocation)
        {
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
            vertexBufferView.BufferLocation = D3DResource()->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = static_cast<uint32>(GetDescription().Width);
            vertexBufferView.StrideInBytes = static_cast<uint32>(GetStride());

            mVertexBufferView = vertexBufferView;
        }

        return mVertexBufferView;
    }

} // namespace Engine::Memory