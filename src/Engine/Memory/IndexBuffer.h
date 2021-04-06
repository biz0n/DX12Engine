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

        const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;

        uint32 GetElementsCount() const { return GetDescription().Width / GetStride(); }

    private:
        mutable D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
    };

} // namespace Engine::Memory