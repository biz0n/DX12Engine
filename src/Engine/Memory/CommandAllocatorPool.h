#pragma once

#include <Types.h>

#include <d3d12.h>
#include <map>
#include <queue>

namespace Engine::Memory
{
    class CommandAllocatorPool
    {
    public:
        CommandAllocatorPool(ComPtr<ID3D12Device> device, Size initialSize);
        ~CommandAllocatorPool();

    public:
        ComPtr<ID3D12CommandAllocator> GetNextAllocator(D3D12_COMMAND_LIST_TYPE type);
        void Reset();

    private:
        ComPtr<ID3D12CommandAllocator> CreateAllocator(D3D12_COMMAND_LIST_TYPE type);
        void PopulateAllocators(D3D12_COMMAND_LIST_TYPE type, Size initialSize);

    private:
        using AllocatorQueue = std::queue<ComPtr<ID3D12CommandAllocator>>;
        using AllocatorsQueueMap = std::map<D3D12_COMMAND_LIST_TYPE, AllocatorQueue>;

        AllocatorsQueueMap mAllocatorsInUse;
        AllocatorsQueueMap mFreeAllocators;

        ComPtr<ID3D12Device> mDevice;
    };
} // namespace Engine::Memory