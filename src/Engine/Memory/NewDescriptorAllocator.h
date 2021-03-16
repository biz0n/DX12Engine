#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorHeap.h>
#include <Memory/IndexPool.h>

#include <d3d12.h>

namespace Engine::Memory
{
    struct Descriptor
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuAddress;
        D3D12_GPU_DESCRIPTOR_HANDLE GpuAddress;
        Index HeapIndex;
    };

    struct DescriptorAllocatorConfig
    {
        Size RenderTargetHeapSize = 64;
        Size DepthStencilHeapSize = 32;
        Size ShaderResourceViewRange = 1024;
        Size ConstantBufferViewRange = 1024;
        Size UnorderedAccessViewRange = 1024;
        Size SamplerHeapSize = 64;
        static constexpr Index SRVRange = 0;
        static constexpr Index CBVRange = 1;
        static constexpr Index UAVRange = 2;
    };

    class NewDescriptorAllocator
    {
        public:
            NewDescriptorAllocator(ID3D12Device* device, DescriptorAllocatorConfig heapSizes);
            ~NewDescriptorAllocator();

        public:
            Descriptor AllocateRTDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice = 0);
            Descriptor AllocateDSDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice = 0);
            Descriptor AllocateSRDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize);
            Descriptor AllocateSRCubeDescriptor(ID3D12Resource* resource, Index heapIndex);
            Descriptor AllocateSRRaytracingASDescriptor(ID3D12Resource* resource, Index heapIndex);
            Descriptor AllocateCBDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize);
            Descriptor AllocateUADescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize, uint32 mipSlice = 0);
            Descriptor AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc, Index heapIndex);

        public:
            const DescriptorHeap& GetSrvCbvUavHeap() const { return mSrvCbvUavHeap; }
            const DescriptorHeap& GetSamplerHeap() const { return mSamplerHeap; }

        private:
            DescriptorHeap mRTHeap;
            DescriptorHeap mDSHeap;
            DescriptorHeap mSrvCbvUavHeap;
            DescriptorHeap mSamplerHeap;

        private:
            ID3D12Device* mDevice;
    };
}