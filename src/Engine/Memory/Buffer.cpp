//
// Created by Maxim on 3/17/2021.
//

#include "Buffer.h"
#include <Memory/ResourceFactory.h>
#include <Memory/UploadBuffer.h>
#include <Render/ResourceStateTracker.h>

void Engine::Memory::ResourceCopyData::Execute(ID3D12GraphicsCommandList *copyCommandList) const
{
    CopyCommand(copyCommandList, UploadBuffer->D3DResource(), DestinationResource);
}


    Engine::Memory::Buffer::Buffer(ID3D12Device *device, ResourceAllocator *resourceFactory,
                                   Engine::Memory::DescriptorAllocatorPool *descriptorAllocator,
                                   Engine::Render::GlobalResourceStateTracker *stateTracker,
                                   Size stride,
                                   D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType)
            : mDescriptorAllocator{descriptorAllocator}, mStateTracker{stateTracker}
    {
        assert_format(desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER, "Unsupported dimension for buffer");

        switch (heapType)
        {
            case D3D12_HEAP_TYPE_UPLOAD:
                state = D3D12_RESOURCE_STATE_GENERIC_READ;
                break;
            case D3D12_HEAP_TYPE_READBACK:
                state = D3D12_RESOURCE_STATE_COPY_DEST;
                break;
            default:
                break;
        }

        const int GPUMask = 0;
        auto allocationInfo = device->GetResourceAllocationInfo(GPUMask, 1, &desc);

        desc.Alignment = allocationInfo.Alignment;
        mDescription = desc;
        mSize = desc.Width;
        mAlignment = allocationInfo.Alignment;
        mStride = stride;

        mResource = resourceFactory->CreateResource(desc, state, heapType);

        stateTracker->TrackResource(mResource.Get(), state);
    }

    Engine::Memory::Buffer::~Buffer()
    {
        mStateTracker->UntrackResource(mResource.Get());
    }

    const Engine::Memory::DescriptorAllocation &Engine::Memory::Buffer::GetCBDescriptor() const
    {
        if (mCBDescriptor.IsNull())
        {
            mCBDescriptor = mDescriptorAllocator->AllocateCBDescriptor(D3DResource(), mStride);
        }

        return mCBDescriptor;
    }

    const Engine::Memory::DescriptorAllocation &Engine::Memory::Buffer::GetSRDescriptor() const
    {
        if (mSRDescriptor.IsNull())
        {
            mSRDescriptor = mDescriptorAllocator->AllocateSRDescriptor(D3DResource(), mStride);
        }
        return mSRDescriptor;
    }

    const Engine::Memory::DescriptorAllocation &Engine::Memory::Buffer::GetUADescriptor() const
    {
        if (mUADescriptor.IsNull())
        {
            mUADescriptor = mDescriptorAllocator->AllocateUADescriptor(D3DResource(), mStride);
        }

        return mUADescriptor;
    }

    Size Engine::Memory::Buffer::GetSubresourcesCount() const
    {
        return 1;
    }

    Engine::Memory::CopyCommandFunction Engine::Memory::Buffer::GetCopyCommandFunction() const
    {
        Memory::CopyCommandFunction copy = [](ID3D12GraphicsCommandList *commandList, ID3D12Resource *source,
                                              ID3D12Resource *destination)
        {
            commandList->CopyBufferRegion(destination, 0, source, 0, destination->GetDesc().Width);
        };

        return copy;
    }

    void Engine::Memory::Buffer::ScheduleUploading(Engine::Memory::ResourceFactory *resourceFactory, Engine::Memory::ResourceCopyManager *copyManager,
                                                   Engine::Memory::Buffer *buffer, const void *data, Size size, D3D12_RESOURCE_STATES finalState)
    {
        auto uploadBuffer = resourceFactory->CreateUploadBuffer(size);

        Memory::ResourceCopyData copyData{};
        copyData.DestinationResource = buffer->D3DResource();
        copyData.UploadBuffer = uploadBuffer;
        copyData.StateAfterCopy = finalState;
        copyData.CopyCommand = buffer->GetCopyCommandFunction();

        auto indicesAllocation = uploadBuffer->Allocate(size, 1u);
        indicesAllocation.CopyTo(data, size);

        copyManager->ScheduleWriting(copyData);
    }


