#include "DescriptorHeap.h"

#include <Exceptions.h>

#include <d3dx12.h>
#include <numeric>

namespace Engine::HAL
{
    DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, std::vector<Size> ranges)
    {
        Size size = std::accumulate(ranges.begin(), ranges.end(), 0);
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = type;
        desc.NumDescriptors = static_cast<uint32>(size);
        desc.NodeMask = 0;
        bool shaderVisible = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap)));

        mDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(type);
        mCpu = mHeap->GetCPUDescriptorHandleForHeapStart();
        mGpu = {};

        if (shaderVisible)
        {
            mGpu = mHeap->GetGPUDescriptorHandleForHeapStart();
        }

        mHeap->SetName((L"DescriptorAllocatorPool heap: " + std::to_wstring(type)).c_str());

        Size offset = 0;
        for (auto range : ranges)
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE cpu(mCpu, static_cast<int32>(offset), mDescriptorHandleIncrementSize);
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpu(mGpu, static_cast<int32>(offset), mDescriptorHandleIncrementSize);
            mRanges.emplace_back(HeapRange{cpu, gpu, range});
            offset += range;
        }
    }

    DescriptorHeap::~DescriptorHeap() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuAddress(Index index, Index range) const
    {
        auto rangeData = GetRange(range);
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rangeData.CpuAddress, static_cast<int32>(index), mDescriptorHandleIncrementSize);
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuAddress(Index index, Index range) const
    {
        auto rangeData = GetRange(range);
        CD3DX12_GPU_DESCRIPTOR_HANDLE handle(rangeData.GpuAddress, static_cast<int32>(index), mDescriptorHandleIncrementSize);
        return handle;
    }
}