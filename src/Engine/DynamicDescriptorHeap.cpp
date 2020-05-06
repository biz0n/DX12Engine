#include "DynamicDescriptorHeap.h"

#include "Exceptions.h"

#include <stdexcept>

namespace Engine
{

    DynamicDescriptHeap::DynamicDescriptHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 descriptorHandleIncrementSize, uint32 descriptorsPerHeap)
        : mDescriptorHeapType(heapType), mDescriptorHandleIncrementSize(descriptorHandleIncrementSize), mDescriptorsPerHeap(descriptorsPerHeap)
    {
        mDescriptorHandlesCache = MakeUnique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(descriptorsPerHeap);
    }

    void DynamicDescriptHeap::StageDescriptor(uint32 rootParameterIndex, uint32 offset, uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        if (numDescriptors > mDescriptorsPerHeap || rootParameterIndex > RootSignature::MaxDescriptorTables)
        {
            throw std::bad_alloc();
        }

        DescriptorTableCache &descriptorTableCache = mDescriptoTableCache[rootParameterIndex];

        if ((offset + numDescriptors) > descriptorTableCache.NumDescriptors)
        {
            throw std::length_error("");
        }

        D3D12_CPU_DESCRIPTOR_HANDLE *dstHandle = (descriptorTableCache.BaseDescriptor + offset);

        for (uint32 i = 0; i < numDescriptors; ++i)
        {
            dstHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptor, i, mDescriptorHandleIncrementSize);
        }

        mStaleDescriptorsTableBitMask |= (1 << rootParameterIndex);
    }

    void DynamicDescriptHeap::ParseRootSignature(const RootSignature *rootSignature)
    {
        mStaleDescriptorsTableBitMask = 0;

        mDescriptorsTableBitMask = rootSignature->GetDescriptorsBitMask(mDescriptorHeapType);
        auto bitMask = mDescriptorsTableBitMask;
        DWORD index;

        uint32 offset = 0;
        while (_BitScanForward(&index, bitMask) && index < rootSignature->GetRootParametersCount())
        {
            uint32 numDescriptorsPerTable = rootSignature->GetNumDescriptorsPerTable(index);

            DescriptorTableCache &descriptorTableCache = mDescriptoTableCache[index];
            descriptorTableCache.NumDescriptors = numDescriptorsPerTable;
            descriptorTableCache.BaseDescriptor = mDescriptorHandlesCache.get() + offset;

            offset += numDescriptorsPerTable;

            bitMask ^= (1 << index);
        }
    }

    void DynamicDescriptHeap::CommitStagedDescriptors(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        if (mCurrentDescriptorHeap == nullptr || mNumFreeHandles < ComputeStaleDescriptorCount())
        {
            mCurrentDescriptorHeap = GetDescriptorHeap(device);
            mCurrentCpuHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            mCurrentGpuHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            mNumFreeHandles = mDescriptorsPerHeap;

            ID3D12DescriptorHeap *heaps[] = {mCurrentDescriptorHeap.Get()};
            commandList->SetDescriptorHeaps(1, heaps);
        }

        DWORD index;
        while (_BitScanForward(&index, mStaleDescriptorsTableBitMask))
        {
            DescriptorTableCache &descriptorTableCache = mDescriptoTableCache[index];
            uint32 numDescriptors = descriptorTableCache.NumDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE *srcDescriptor = descriptorTableCache.BaseDescriptor;

            D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor[]{mCurrentCpuHandle};

            uint32 dstSizes[]{numDescriptors};

            device->CopyDescriptors(1, dstDescriptor, dstSizes, numDescriptors, srcDescriptor, nullptr, mDescriptorHeapType);

            commandList->SetGraphicsRootDescriptorTable(index, mCurrentGpuHandle);

            mCurrentCpuHandle.Offset(numDescriptors, mDescriptorHandleIncrementSize);
            mCurrentGpuHandle.Offset(numDescriptors, mDescriptorHandleIncrementSize);

            mNumFreeHandles -= numDescriptors;

            mStaleDescriptorsTableBitMask ^= (1 << index);
        }
    }

    void DynamicDescriptHeap::Reset()
    {
        mFreeDescriptorHeaps = mDescriptorHeapPool;
        mCurrentDescriptorHeap.Reset();

        mCurrentCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
        mCurrentGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);

        mStaleDescriptorsTableBitMask = 0;
        mDescriptorsTableBitMask = 0;

        mNumFreeHandles = 0;
    }

    ComPtr<ID3D12DescriptorHeap> DynamicDescriptHeap::GetDescriptorHeap(ComPtr<ID3D12Device> device)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;
        if (!mFreeDescriptorHeaps.empty())
        {
            descriptorHeap = mFreeDescriptorHeaps.front();
            mFreeDescriptorHeaps.pop();
        }
        else
        {
            descriptorHeap = CreateDescriptorHeap(device);
            mDescriptorHeapPool.push(descriptorHeap);
        }

        return descriptorHeap;
    }

    ComPtr<ID3D12DescriptorHeap> DynamicDescriptHeap::CreateDescriptorHeap(ComPtr<ID3D12Device> device)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        heapDesc.Type = mDescriptorHeapType;
        heapDesc.NumDescriptors = mDescriptorsPerHeap;
        heapDesc.NodeMask = 0;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)));

        return descriptorHeap;
    }

    uint32 DynamicDescriptHeap::ComputeStaleDescriptorCount() const
    {
        DWORD index;
        DWORD staleDescriptorTableBitMask = mStaleDescriptorsTableBitMask;

        uint32 staleDescriptorsCount = 0;

        while (_BitScanForward(&index, staleDescriptorTableBitMask))
        {
            staleDescriptorsCount += mDescriptoTableCache[index].NumDescriptors;
            staleDescriptorTableBitMask ^= (1 << index);
        }

        return staleDescriptorsCount;
    }

} // namespace Engine