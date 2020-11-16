#include "UploadBuffer.h"
#include <Exceptions.h>
#include <d3dx12.h>
#include <new>

namespace Engine::Memory
{
    UploadBuffer::UploadBuffer(ID3D12Device *device, Size size)
        : mOffset(0), mSize(size)
    {
        CD3DX12_HEAP_PROPERTIES properties {D3D12_HEAP_TYPE_UPLOAD};
        D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
        ThrowIfFailed(device->CreateCommittedResource(
            &properties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mBuffer)));
        ThrowIfFailed(mBuffer->Map(0, nullptr, reinterpret_cast<void **>(&mMappedData)));

        mBuffer->SetName(L"Upload Buffer");
        mGpuAddress = mBuffer->GetGPUVirtualAddress();
    }

    UploadBuffer::~UploadBuffer()
    {
        if (mBuffer != nullptr)
        {
            mBuffer->Unmap(0, nullptr);
        }
        mGpuAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);
        mMappedData = nullptr;
    }

    UploadBuffer::Allocation UploadBuffer::Allocate(Size sizeInBytes, Size alignment)
    {
        Size alignedSize = (sizeInBytes + (alignment - 1)) & ~(alignment - 1);
        Size alignedOffset = (mOffset + (alignment - 1)) & ~(alignment - 1);

        if (alignedOffset + alignedSize > mSize)
        {
            throw std::bad_alloc();
        }

        Allocation allocation;
        allocation.CPU = mMappedData + alignedOffset;
        allocation.GPU = mGpuAddress + alignedOffset;
        allocation.bufferSize = alignedSize;
        allocation.offset = alignedOffset;

        mOffset = alignedOffset + alignedSize;

        return allocation;
    }

} // namespace Engine::Memory