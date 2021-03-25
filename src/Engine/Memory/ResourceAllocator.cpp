//
// Created by Maxim on 3/22/2021.
//

#include "ResourceAllocator.h"

namespace Engine::Memory
{

    ResourceAllocator::ResourceAllocator(ID3D12Device *device) : mDevice(device)
    {
    }

    ResourceAllocator::~ResourceAllocator() = default;

    ComPtr<ID3D12Resource>
    ResourceAllocator::CreateResource(const D3D12_RESOURCE_DESC &desc, D3D12_RESOURCE_STATES state,
                                      D3D12_HEAP_TYPE heapType,
                                      const D3D12_CLEAR_VALUE *clearValue)
    {
        CD3DX12_HEAP_PROPERTIES heapProperties{heapType};

        ComPtr<ID3D12Resource> resource;
        ThrowIfFailed(mDevice->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                state,
                clearValue,
                IID_PPV_ARGS(&resource)
        ));

        return resource;
    }


}

