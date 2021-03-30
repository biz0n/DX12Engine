#pragma once

#include <Types.h>
#include <Memory/Buffer.h>
#include <Memory/ResourceAllocator.h>

namespace Engine::Memory
{
    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(ID3D12Device *device,
                    ResourceAllocator *resourceFactory,
                    DescriptorAllocatorPool *descriptorAllocator,
                    Engine::Memory::GlobalResourceStateTracker* stateTracker,
                    Size elementsCount,
                    Size stride,
                    D3D12_RESOURCE_STATES state);
        ~IndexBuffer() override ;

        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

        uint32 GetElementsCount() const { return GetDescription().Width / GetStride(); }

    private:
        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    };

} // namespace Engine::Memory