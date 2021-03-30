#pragma once

#include <Types.h>
#include <Exceptions.h>
#include <vector>
#include <d3d12.h>
#include <d3dx12.h>


namespace Engine::Memory
{
    class ResourceAllocator
    {
    public:
        ResourceAllocator(ID3D12Device *device);
        ~ResourceAllocator();

        ComPtr<ID3D12Resource> CreateResource(
                const D3D12_RESOURCE_DESC &desc,
                D3D12_RESOURCE_STATES state,
                D3D12_HEAP_TYPE heapType,
                const D3D12_CLEAR_VALUE *clearValue = nullptr);


    private:

        ID3D12Device *mDevice;

    };

}