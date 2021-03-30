#pragma once

#include <Types.h>
#include <Memory/Buffer.h>
#include <Memory/ResourceAllocator.h>
#include <d3d12.h>

namespace Engine::Memory
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(ID3D12Device *device,
                     ResourceAllocator *resourceFactory,
                     DescriptorAllocatorPool *descriptorAllocator,
                     Engine::Memory::GlobalResourceStateTracker* stateTracker,
                     Size elementsCount,
                     Size stride,
                     D3D12_RESOURCE_STATES state);
        ~VertexBuffer() ;

        D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();

    private:
        D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
    };

} // namespace Engine::Memory