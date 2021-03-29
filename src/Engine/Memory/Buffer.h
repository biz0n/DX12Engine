#pragma once

#include <Types.h>
#include <Exceptions.h>
#include <Memory/Resource.h>
#include <Memory/DescriptorAllocatorPool.h>
#include <Memory/ResourceAllocator.h>
#include <Render/RenderForwards.h>
#include <Memory/ResourceCopyManager.h>

#include <vector>
#include <d3d12.h>
#include <d3dx12.h>

namespace Engine::Memory
{
    class Buffer : public Resource
    {
    public:
        Buffer(ID3D12Device *device,
               ResourceAllocator *resourceFactory,
               DescriptorAllocatorPool *descriptorAllocator,
               Engine::Render::GlobalResourceStateTracker* stateTracker,
               Size stride,
               D3D12_RESOURCE_DESC desc,
               D3D12_RESOURCE_STATES state,
               D3D12_HEAP_TYPE heapType);

        ~Buffer() override;

        const DescriptorAllocation &GetCBDescriptor() const;

        const DescriptorAllocation &GetSRDescriptor() const;

        const DescriptorAllocation &GetUADescriptor() const;

        Size GetStride() const { return mStride; }

        Size GetAlignment() const { return mAlignment; }

        Size GetSubresourcesCount() const override;

        CopyCommandFunction GetCopyCommandFunction() const override;

        static void ScheduleUploading(
                ResourceFactory *resourceFactory,
                ResourceCopyManager* copyManager,
                Buffer* buffer,
                const void* data,
                Size size,
                D3D12_RESOURCE_STATES finalState);

    private:
        DescriptorAllocatorPool *mDescriptorAllocator;
        Engine::Render::GlobalResourceStateTracker* mStateTracker;
        Size mStride;
        Size mAlignment;
        Size mSize;

        mutable DescriptorAllocation mCBDescriptor;
        mutable DescriptorAllocation mSRDescriptor;
        mutable DescriptorAllocation mUADescriptor;
    };
}