#pragma once

#include <Types.h>
#include <Render/RootSignature.h>
#include <d3dx12.h>

#include <d3d12.h>
#include <queue>

namespace Engine::Memory
{
    class DynamicDescriptorHeap
    {
    public:
        DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 descriptorHandleIncrementSize, uint32 descriptorsPerHeap = 1024);

        void StageDescriptor(uint32 rootParameterIndex, uint32 offset, uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        void ParseRootSignature(const Render::RootSignature *rootSignature);

        void CommitStagedDescriptors(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

        void Reset();

    private:
        ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(ComPtr<ID3D12Device> device);

        ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device> device);

        uint32 ComputeStaleDescriptorCount() const;

        struct DescriptorTableCache
        {
            DescriptorTableCache()
                : numDescriptors(0), baseDescriptor(nullptr)
            {
            }

            void Reset()
            {
                numDescriptors = 0;
                baseDescriptor = nullptr;
            }

            uint32 numDescriptors;
            D3D12_CPU_DESCRIPTOR_HANDLE *baseDescriptor;
        };

        UniquePtr<D3D12_CPU_DESCRIPTOR_HANDLE[]> mDescriptorHandlesCache;
        DescriptorTableCache mDescriptoTableCache[Render::RootSignature::MaxDescriptorTables];

        D3D12_DESCRIPTOR_HEAP_TYPE mDescriptorHeapType;
        uint32 mDescriptorHandleIncrementSize;
        uint32 mDescriptorsPerHeap;

        uint32 mStaleDescriptorsTableBitMask;
        uint32 mDescriptorsTableBitMask;

        std::queue<ComPtr<ID3D12DescriptorHeap>> mDescriptorHeapPool;
        std::queue<ComPtr<ID3D12DescriptorHeap>> mFreeDescriptorHeaps;

        ComPtr<ID3D12DescriptorHeap> mCurrentDescriptorHeap;
        CD3DX12_CPU_DESCRIPTOR_HANDLE mCurrentCpuHandle;
        CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrentGpuHandle;
        uint32 mNumFreeHandles;
    };

} // namespace Engine::Memory