#pragma once

#include <Types.h>

#include <d3d12.h>

namespace Engine
{
    class CommandQueue
    {
    public:
        CommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
        ~CommandQueue();

    public:
        ID3D12CommandQueue* D3D12CommandQueue() const { return mCommandQueue.Get(); }

        uint64 ExecuteCommandList(ID3D12GraphicsCommandList *commandList);

        uint64 ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { return ExecuteCommandList(commandList.Get()); }

        bool IsFenceCompleted(uint64 fenceValue);

        void InsertWait(uint64 fenceValue);
        void InsertWaitForQueue(SharedPtr<CommandQueue> otherQueue, uint64 fencValue);
        void InsertWaitForQueue(SharedPtr<CommandQueue> otherQueue);

        uint64 GetFenceValue() const { return mFenceValue; }
        uint64 GetNextFenceValue() const { return mFenceNextValue; }
        ComPtr<ID3D12Fence> GetFence() const { return mFence; }

        void WaitForFenceCPU(uint64 fenceValue);
        void WaitForIdle();
        void Flush();

    private:
        ComPtr<ID3D12CommandQueue> mCommandQueue;

        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;
        uint64 mFenceValue;
        uint64 mFenceNextValue;
    };

} // namespace Engine