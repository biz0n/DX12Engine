#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>

#include <Render/RenderForwards.h>

#include <d3d12.h>

namespace Engine::Memory
{

    class ResourceFactory
    {
    public:
        ResourceFactory(ID3D12Device *device,
                        ResourceAllocator *resourceAllocator,
                        DescriptorAllocatorPool *descriptorAllocator,
                        Engine::Render::GlobalResourceStateTracker *resourceStateTracker);

        SharedPtr<Texture> CreateTexture(D3D12_RESOURCE_DESC desc,
                                         D3D12_RESOURCE_STATES state,
                                         const D3D12_CLEAR_VALUE *clearValue = nullptr);

        SharedPtr<Texture> CreateTexture(ComPtr<ID3D12Resource> resource,
                                         D3D12_RESOURCE_STATES state);

        SharedPtr<Buffer> CreateBuffer(Size stride,
                                       D3D12_RESOURCE_DESC desc,
                                       D3D12_RESOURCE_STATES state);

        SharedPtr<Engine::Memory::VertexBuffer> CreateVertexBuffer(Size elementsCount, Size elementSize, D3D12_RESOURCE_STATES state);

        SharedPtr<IndexBuffer> CreateIndexBuffer(Size elementsCount, Size elementSize, D3D12_RESOURCE_STATES state);

        SharedPtr<UploadBuffer> CreateUploadBuffer(Size size);

    private:
        ID3D12Device *mDevice;
        ResourceAllocator *mResourceAllocator;
        DescriptorAllocatorPool *mDescriptorAllocator;
        Engine::Render::GlobalResourceStateTracker *mResourceStateTracker;
    };
}