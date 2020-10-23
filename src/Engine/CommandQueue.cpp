#include "CommandQueue.h"

#include <Exceptions.h>

namespace Engine
{
    CommandQueue::CommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue)));

        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

        mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        mFenceValue = ((uint64)type << 56);
        mFenceNextValue = mFenceValue + 1;

        mFence->Signal(mFenceValue);
    }

    CommandQueue::~CommandQueue()
    {
        ::CloseHandle(mFenceEvent);
    }

    uint64 CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList *commandList)
    {
        ThrowIfFailed(commandList->Close());

        ID3D12CommandList *const commandLists[] = {commandList};

        mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mFenceNextValue));

        return mFenceNextValue++;
    }

    bool CommandQueue::IsFenceCompleted(uint64 fenceValue)
    {
        if (fenceValue > mFenceValue)
        {
            mFenceValue = max(mFenceValue, mFence->GetCompletedValue());
        }

        return fenceValue <= mFenceValue;
    }

    void CommandQueue::InsertWait(uint64 fenceValue)
    {
        mCommandQueue->Wait(mFence.Get(), fenceValue);
    }

    void CommandQueue::InsertWaitForQueue(SharedPtr<CommandQueue> otherQueue, uint64 fenceValue)
    {
        mCommandQueue->Wait(otherQueue->GetFence().Get(), fenceValue);
    }

    void CommandQueue::InsertWaitForQueue(SharedPtr<CommandQueue> otherQueue)
    {
        mCommandQueue->Wait(otherQueue->GetFence().Get(), otherQueue->GetNextFenceValue() - 1);
    }

    void CommandQueue::WaitForFenceCPU(uint64 fenceValue)
    {
        if (IsFenceCompleted(fenceValue))
        {
            return;
        }

        mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
        ::WaitForSingleObject(mFenceEvent, INFINITE);
        mFenceValue = fenceValue;
    }

    void CommandQueue::WaitForIdle()
    {
        WaitForFenceCPU(mFenceNextValue - 1);
    }

    void CommandQueue::Flush()
    {
        mCommandQueue->Signal(mFence.Get(), mFenceNextValue);
        mFenceNextValue++;
        WaitForIdle();
    }
}