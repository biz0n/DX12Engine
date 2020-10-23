#include "CommandAllocatorPool.h"

#include <Exceptions.h>

namespace Engine
{
    CommandAllocatorPool::CommandAllocatorPool(ComPtr<ID3D12Device> device, Size initialSize) : mDevice(device)
    {
        PopulateAllocators(D3D12_COMMAND_LIST_TYPE_DIRECT, initialSize);
        PopulateAllocators(D3D12_COMMAND_LIST_TYPE_COMPUTE, initialSize);
        PopulateAllocators(D3D12_COMMAND_LIST_TYPE_COPY, initialSize);
    }

    CommandAllocatorPool::~CommandAllocatorPool() = default;

    ComPtr<ID3D12CommandAllocator> CommandAllocatorPool::GetNextAllocator(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> allocator;

        auto &queue = mFreeAllocators[type];
        if (queue.empty())
        {
            allocator = CreateAllocator(type);
        }
        else
        {
            allocator = queue.front();
            queue.pop();
        }
        mAllocatorsInUse[type].push(allocator);

        return allocator;
    }

    void CommandAllocatorPool::Reset()
    {
        for (auto &allocatorsQueue : mAllocatorsInUse)
        {
            auto &freeAllocatorsQueue = mFreeAllocators[allocatorsQueue.first];
            while (!allocatorsQueue.second.empty())
            {
                auto allocator = allocatorsQueue.second.front();
                allocator->Reset();
                allocatorsQueue.second.pop();
                freeAllocatorsQueue.push(allocator);
            }
        }
    }

    ComPtr<ID3D12CommandAllocator> CommandAllocatorPool::CreateAllocator(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;

        ThrowIfFailed(mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

        return commandAllocator;
    }

    void CommandAllocatorPool::PopulateAllocators(D3D12_COMMAND_LIST_TYPE type, Size initialSize)
    {
        for (Size i = 0; i < initialSize; ++i)
        {
            mFreeAllocators[type].push(CreateAllocator(type));
        }
    }
}