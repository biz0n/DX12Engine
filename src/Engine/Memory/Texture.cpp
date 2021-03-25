//
// Created by Maxim on 3/22/2021.
//

#include "Texture.h"
#include <Memory/ResourceFactory.h>
#include <Render/ResourceStateTracker.h>

Engine::Memory::Texture::Texture(ID3D12Device *device, ResourceAllocator *resourceAllocator,
                                 Engine::Memory::DescriptorAllocatorPool *descriptorAllocator,
                                 Engine::Render::GlobalResourceStateTracker* stateTracker,
                                 D3D12_RESOURCE_DESC desc, const D3D12_CLEAR_VALUE *clearValue,
                                 D3D12_RESOURCE_STATES state)
                                 : mDescriptorAllocator{descriptorAllocator}, mStateTracker{stateTracker}, 
                                   mCBDescriptor{},
                                   mSRDescriptor{},
                                   mCubeSRDescriptor{}
{

    assert_format(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER, "Unsupported dimension for buffer");

    const int GPUMask = 0;
    auto allocationInfo = device->GetResourceAllocationInfo(GPUMask, 1, &desc);
    desc.Alignment = allocationInfo.Alignment;
    mSize = desc.Width;
    mAlignment = allocationInfo.Alignment;

    mResource = resourceAllocator->CreateResource(desc, state, D3D12_HEAP_TYPE_DEFAULT, clearValue);
    mDescription = desc;

    mUADescriptor.resize(desc.MipLevels);
    mRTDescriptor.resize(desc.MipLevels);
    mDSDescriptor.resize(desc.MipLevels);

    stateTracker->TrackResource(mResource.Get(), state);
}

Engine::Memory::Texture::Texture(ComPtr<ID3D12Resource> resource,
                                 Engine::Memory::DescriptorAllocatorPool *descriptorAllocator,
                                 Engine::Render::GlobalResourceStateTracker* stateTracker,
                                 D3D12_RESOURCE_STATES state)
        : mDescriptorAllocator{descriptorAllocator}, mStateTracker{stateTracker},
          mCBDescriptor{}, mSRDescriptor{}, mCubeSRDescriptor{}
{
    mResource = resource;
    auto desc = resource->GetDesc();
    assert_format(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER, "Unsupported dimension for buffer");

    mDescription = desc;
    mSize = desc.Width;
    mAlignment = desc.Alignment;

    mUADescriptor.resize(desc.MipLevels);
    mRTDescriptor.resize(desc.MipLevels);
    mDSDescriptor.resize(desc.MipLevels);

    stateTracker->TrackResource(mResource.Get(), state);
}


Engine::Memory::Texture::~Texture()
{
    mStateTracker->UntrackResource(mResource.Get());
}


Size Engine::Memory::Texture::GetSubresourcesCount() const
{
    auto arraySize = (mDescription.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D || mDescription.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) ? mDescription.DepthOrArraySize : 1;
    auto numSubresources = arraySize * mDescription.MipLevels;
    return numSubresources;
}

Engine::Memory::CopyCommandFunction Engine::Memory::Texture::GetCopyCommandFunction() const
{
    Memory::CopyCommandFunction copy = [](ID3D12GraphicsCommandList *commandList, ID3D12Resource *source, ID3D12Resource* destination)
    {
        auto desc = destination->GetDesc();
        auto arraySize = (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D ||
                          desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
                                  ? desc.DepthOrArraySize : 1;
        auto numSubresources = arraySize * desc.MipLevels;

        uint64 requiredSize = 0;
        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
        std::vector<uint32> numRows;
        std::vector<uint64> rowSizesInBytes;

        layouts.resize(numSubresources);
        numRows.resize(numSubresources);
        rowSizesInBytes.resize(numSubresources);

        ComPtr<ID3D12Device> device;
        destination->GetDevice(IID_PPV_ARGS(&device));

        device->GetCopyableFootprints(
                &desc,
                0,
                numSubresources,
                0,
                layouts.data(),
                numRows.data(),
                rowSizesInBytes.data(),
                &requiredSize);

        for (UINT i = 0; i < numSubresources; ++i)
        {
            CD3DX12_TEXTURE_COPY_LOCATION Dst(destination, i);
            CD3DX12_TEXTURE_COPY_LOCATION Src(source, layouts[i]);
            commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        }
    };

    return copy;
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetCBDescriptor() const
{
    if (mCBDescriptor.IsNull())
    {
        mCBDescriptor = mDescriptorAllocator->AllocateCBDescriptor(D3DResource(), 1);
    }

    return mCBDescriptor;
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetSRDescriptor() const
{
    if (mSRDescriptor.IsNull())
    {
        mSRDescriptor = mDescriptorAllocator->AllocateSRDescriptor(D3DResource(), 1);
    }
    return mSRDescriptor;
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetCubeSRDescriptor() const
{
    if (mCubeSRDescriptor.IsNull())
    {
        mCubeSRDescriptor = mDescriptorAllocator->AllocateSRCubeDescriptor(D3DResource());
    }
    return mCubeSRDescriptor;
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetUADescriptor(uint32 mipSlice) const
{
    if (mUADescriptor[mipSlice].IsNull())
    {
        mUADescriptor[mipSlice] = mDescriptorAllocator->AllocateUADescriptor(D3DResource(), 1, mipSlice);
    }

    return mUADescriptor[mipSlice];
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetRTDescriptor(uint32 mipSlice) const
{
    if (mRTDescriptor[mipSlice].IsNull())
    {
        mRTDescriptor[mipSlice] = mDescriptorAllocator->AllocateRTDescriptor(D3DResource(), mipSlice);
    }

    return mRTDescriptor[mipSlice];
}

const Engine::Memory::NewDescriptorAllocation &Engine::Memory::Texture::GetDSDescriptor(uint32 mipSlice) const
{
    if (mDSDescriptor[mipSlice].IsNull())
    {
        mDSDescriptor[mipSlice] = mDescriptorAllocator->AllocateDSDescriptor(D3DResource(), mipSlice);
    }

    return mDSDescriptor[mipSlice];
}

