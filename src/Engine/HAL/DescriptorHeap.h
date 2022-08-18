#pragma once

#include <Types.h>

#include <vector>
#include <d3d12.h>

namespace Engine::HAL
{
    class DescriptorHeap
    {
        public:
            struct HeapRange
            {
                D3D12_CPU_DESCRIPTOR_HANDLE CpuAddress;
                D3D12_GPU_DESCRIPTOR_HANDLE GpuAddress;
                Size Range;
                Size Offset;
            };

        public:
            DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, std::vector<Size> ranges);
            ~DescriptorHeap();

            D3D12_CPU_DESCRIPTOR_HANDLE GetCpuAddress(Index index, Index range) const;

            D3D12_GPU_DESCRIPTOR_HANDLE GetGpuAddress(Index index, Index range) const;

            ID3D12DescriptorHeap* D3DHeap() const { return mHeap.Get(); }

            const HeapRange& GetRange(Index range = 0) const { return mRanges.at(range); }

            const Size GetRangeOffset(Index range) const { return mRanges.at(range).Offset; }

        private:
            ComPtr<ID3D12DescriptorHeap> mHeap;
            uint32 mDescriptorHandleIncrementSize;
            D3D12_CPU_DESCRIPTOR_HANDLE mCpu;
            D3D12_GPU_DESCRIPTOR_HANDLE mGpu;
            std::vector<HeapRange> mRanges;

    };
}