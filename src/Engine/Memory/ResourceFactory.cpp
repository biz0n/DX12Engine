//
// Created by Maxim on 3/22/2021.
//

#include "ResourceFactory.h"

#include <Memory/Texture.h>
#include <Memory/Buffer.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/UploadBuffer.h>

#include <Render/ResourceStateTracker.h>

namespace Engine::Memory
{

    ResourceFactory::ResourceFactory(ID3D12Device *device, ResourceAllocator *resourceAllocator,
                                     DescriptorAllocatorPool *descriptorAllocator,
                                     Engine::Render::GlobalResourceStateTracker *resourceStateTracker) :
            mDevice{device}, mResourceAllocator{resourceAllocator}, mDescriptorAllocator{descriptorAllocator},
            mResourceStateTracker{resourceStateTracker}
    {

    }

    SharedPtr<Texture> ResourceFactory::CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES state,
                                                      const D3D12_CLEAR_VALUE *clearValue)
    {
        auto texture = MakeShared<Texture>(mDevice, mResourceAllocator, mDescriptorAllocator, mResourceStateTracker,
                                           desc,
                                           clearValue, state);

        return texture;
    }

    SharedPtr<Texture> ResourceFactory::CreateTexture(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state)
    {
        auto texture = MakeShared<Texture>(resource, mDescriptorAllocator, mResourceStateTracker, state);

        return texture;
    }

    SharedPtr<Buffer> ResourceFactory::CreateBuffer(Size stride, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES state)
    {
        auto buffer = MakeShared<Buffer>(mDevice, mResourceAllocator, mDescriptorAllocator, mResourceStateTracker,
                                         stride,
                                         desc,
                                         state, D3D12_HEAP_TYPE_DEFAULT);

        return buffer;
    }

    SharedPtr<VertexBuffer>
    ResourceFactory::CreateVertexBuffer(Size elementsCount, Size elementSize, D3D12_RESOURCE_STATES state)
    {
        auto buffer = MakeShared<VertexBuffer>(mDevice, mResourceAllocator, mDescriptorAllocator,
                                               mResourceStateTracker,
                                               elementsCount, elementSize, state);

        return buffer;
    }

    SharedPtr<IndexBuffer> ResourceFactory::CreateIndexBuffer(Size elementsCount, Size elementSize, D3D12_RESOURCE_STATES state)
    {
        auto buffer = MakeShared<IndexBuffer>(mDevice, mResourceAllocator, mDescriptorAllocator,
                                              mResourceStateTracker,
                                              elementsCount, elementSize, state);

        return buffer;
    }

    SharedPtr<UploadBuffer> ResourceFactory::CreateUploadBuffer(Size size)
    {
        auto buffer = MakeShared<UploadBuffer>(mDevice, mResourceAllocator, mDescriptorAllocator,
                                               mResourceStateTracker,
                                               size);

        return buffer;
    }
}