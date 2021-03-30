#include "UploadBuffer.h"

#include <Exceptions.h>
#include <MathUtils.h>
#include <d3dx12.h>
#include <new>

namespace Engine::Memory
{
    UploadBuffer::UploadBuffer(ID3D12Device *device,
                               ResourceAllocator *resourceFactory,
                               DescriptorAllocatorPool *descriptorAllocator,
                               Engine::Memory::GlobalResourceStateTracker* stateTracker,
                               Size size)
        : Buffer(
                device,
                resourceFactory,
                descriptorAllocator,
                stateTracker,
                1,
                CD3DX12_RESOURCE_DESC::Buffer(size),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                D3D12_HEAP_TYPE_UPLOAD),
         mOffset(0),
         mSize(size),
         mMappedData(nullptr)
    {
        mGpuAddress = D3DResource()->GetGPUVirtualAddress();
        ThrowIfFailed(D3DResource()->Map(0, nullptr, reinterpret_cast<void **>(&mMappedData)));
    }

    UploadBuffer::~UploadBuffer()
    {
        if (D3DResource() != nullptr)
        {
            D3DResource()->Unmap(0, nullptr);
        }
        mGpuAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);
        mMappedData = nullptr;
    }

    UploadBuffer::Allocation UploadBuffer::Allocate(Size sizeInBytes, Size alignment)
    {
        Size alignedSize = Math::AlignUp(sizeInBytes, alignment);
        Size alignedOffset = Math::AlignUp(mOffset, alignment);

        if (alignedOffset + alignedSize > mSize)
        {
            throw std::bad_alloc();
        }

        Allocation allocation{};
        allocation.CPU = mMappedData + alignedOffset;
        allocation.GPU = mGpuAddress + alignedOffset;
        allocation.bufferSize = alignedSize;
        allocation.offset = alignedOffset;

        mOffset = alignedOffset + alignedSize;

        return allocation;
    }

} // namespace Engine::Memory