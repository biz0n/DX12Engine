#pragma once

#include <Types.h>
#include <Memory/MemoryForwards.h>
#include <Memory/IndexPool.h>
#include <HAL/DescriptorHeap.h>

#include <d3d12.h>

namespace Engine::Memory
{
    struct Descriptor
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuAddress;
        D3D12_GPU_DESCRIPTOR_HANDLE GpuAddress;
        Index HeapIndex;
        Size Offset;
    };

    struct DescriptorAllocatorConfig
    {
        Size RenderTargetHeapSize = 64;
        Size DepthStencilHeapSize = 64;
        Size ConstantBufferViewRange = 1024;
        Size ShaderResourceViewRange = 4096;
        Size UnorderedAccessViewRange = 1024;
        Size SamplerHeapSize = 64;
        static constexpr Index CBVRange = 0;
        static constexpr Index SRVRange = 1;
        static constexpr Index UAVRange = 2;
    };

    class DescriptorAllocator
    {
        public:
            DescriptorAllocator(ID3D12Device* device, DescriptorAllocatorConfig heapSizes);
            ~DescriptorAllocator();

        public:
            Descriptor AllocateRTDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice = 0);
            Descriptor AllocateDSDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice = 0);
            Descriptor AllocateSRDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize);
            Descriptor AllocateSRDescriptor(Index heapIndex, uint32 strideSize);
            Descriptor AllocateSRCubeDescriptor(ID3D12Resource* resource, Index heapIndex);
            Descriptor AllocateSRRaytracingASDescriptor(ID3D12Resource* resource, Index heapIndex);
            Descriptor AllocateCBDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize);
            Descriptor AllocateUADescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize, uint32 mipSlice = 0);
            Descriptor AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc, Index heapIndex);

        public:
            const HAL::DescriptorHeap& GetSrvCbvUavHeap() const { return mSrvCbvUavHeap; }
            const HAL::DescriptorHeap& GetSamplerHeap() const { return mSamplerHeap; }

        private:
            HAL::DescriptorHeap mRTHeap;
            HAL::DescriptorHeap mDSHeap;
            HAL::DescriptorHeap mSrvCbvUavHeap;
            HAL::DescriptorHeap mSamplerHeap;

        private:
            ID3D12Device* mDevice;
    };
}