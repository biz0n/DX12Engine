#include "DynamicDescriptorHeap.h"

#include <Exceptions.h>

#include <stdexcept>

namespace Engine::Memory
{
    DynamicDescriptorHeap::DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 descriptorHandleIncrementSize, uint32 descriptorsPerHeap)
        : mDescriptorHeapType(heapType), mDescriptorHandleIncrementSize(descriptorHandleIncrementSize), mDescriptorsPerHeap(descriptorsPerHeap)
    {
        mDescriptorHandlesCache = MakeUnique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(descriptorsPerHeap);
    }

    void DynamicDescriptorHeap::StageDescriptor(uint32 rootParameterIndex, uint32 offset, uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        if (numDescriptors > mDescriptorsPerHeap || rootParameterIndex > Render::RootSignature::MaxDescriptorTables)
        {
            throw std::bad_alloc();
        }

        DescriptorTableCache &descriptorTableCache = mDescriptoTableCache[rootParameterIndex];

        if ((offset + numDescriptors) > descriptorTableCache.numDescriptors)
        {
            throw std::length_error("");
        }

        D3D12_CPU_DESCRIPTOR_HANDLE *dstHandle = (descriptorTableCache.baseDescriptor + offset);

        for (uint32 i = 0; i < numDescriptors; ++i)
        {
            dstHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptor, i, mDescriptorHandleIncrementSize);
        }

        mStaleDescriptorsTableBitMask |= (1 << rootParameterIndex);
    }

    void DynamicDescriptorHeap::ParseRootSignature(const Render::RootSignature *rootSignature)
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
            descriptorTableCache.numDescriptors = numDescriptorsPerTable;
            descriptorTableCache.baseDescriptor = mDescriptorHandlesCache.get() + offset;

            offset += numDescriptorsPerTable;

            bitMask ^= (1 << index);
        }
    }

    void DynamicDescriptorHeap::CommitStagedDescriptors(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        if (mCurrentDescriptorHeap == nullptr || mNumFreeHandles < ComputeStaleDescriptorCount())
        {
            mCurrentDescriptorHeap = GetDescriptorHeap(device);
            mCurrentCpuHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            mCurrentGpuHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            mNumFreeHandles = mDescriptorsPerHeap;

                        //commandList->SetDescriptorHeaps(1, heaps);
        }

        /// TODO: this solution not optimize but for now it's ok
        ID3D12DescriptorHeap *heaps[] = {mCurrentDescriptorHeap.Get()};
        commandList->SetDescriptorHeaps(1, heaps);

        DWORD index;
        while (_BitScanForward(&index, mStaleDescriptorsTableBitMask))
        {
            DescriptorTableCache &descriptorTableCache = mDescriptoTableCache[index];
            uint32 numDescriptors = descriptorTableCache.numDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE *srcDescriptor = descriptorTableCache.baseDescriptor;

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

    void DynamicDescriptorHeap::Reset()
    {
        mFreeDescriptorHeaps = mDescriptorHeapPool;
        mCurrentDescriptorHeap.Reset();

        mCurrentCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
        mCurrentGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);

        mStaleDescriptorsTableBitMask = 0;
        mDescriptorsTableBitMask = 0;

        mNumFreeHandles = 0;
    }

    ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::GetDescriptorHeap(ComPtr<ID3D12Device> device)
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

    ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap(ComPtr<ID3D12Device> device)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        heapDesc.Type = mDescriptorHeapType;
        heapDesc.NumDescriptors = mDescriptorsPerHeap;
        heapDesc.NodeMask = 0;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)));

        descriptorHeap->SetName((L"DynamicDescriptor heap: " + std::to_wstring(mDescriptorHeapType)).c_str());

        return descriptorHeap;
    }

    uint32 DynamicDescriptorHeap::ComputeStaleDescriptorCount() const
    {
        DWORD index;
        DWORD staleDescriptorTableBitMask = mStaleDescriptorsTableBitMask;

        uint32 staleDescriptorsCount = 0;

        while (_BitScanForward(&index, staleDescriptorTableBitMask))
        {
            staleDescriptorsCount += mDescriptoTableCache[index].numDescriptors;
            staleDescriptorTableBitMask ^= (1 << index);
        }

        return staleDescriptorsCount;
    }

} // namespace Engine::Memory