#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>

#include <vector>
#include <tuple>
#include <queue>
#include <functional>

#include <d3d12.h>
#include <d3d12.h>

namespace Engine::Memory
{
    using CopyCommandFunction = std::function<void(ID3D12GraphicsCommandList *, ID3D12Resource *source, ID3D12Resource* destination)>;

    struct ResourceCopyData
    {
        SharedPtr<UploadBuffer> UploadBuffer;
        ID3D12Resource *DestinationResource;
        CopyCommandFunction CopyCommand;
        D3D12_RESOURCE_STATES StateAfterCopy;

        void Execute(ID3D12GraphicsCommandList *copyCommandList) const;
    };


    class ResourceCopyManager
    {
    public:
        struct WriteAllocation
        {
            UniquePtr<Engine::Memory::UploadBuffer> UploadBuffer;
            Byte *CPU;

        };
    public:
        ResourceCopyManager() : mCurrentFrame{0}
        {

        }

        void FrameStarted(uint64 frameNumber)
        {
            mCurrentFrame = frameNumber;
        }

        void FrameEnded()
        {
            while (!mWritingsInProgress.empty() && std::get<uint64>(mWritingsInProgress.front()) <= mCurrentFrame)
            {
                mWritingsInProgress.pop();
            }
        }

        void ScheduleWriting(const ResourceCopyData& copyData)
        {
            mScheduledWritingData.emplace_back(copyData, mCurrentFrame);
        }

        bool Copy(ID3D12GraphicsCommandList* commandList, Memory::ResourceStateTracker* stateTracker);

    private:

        uint64 mCurrentFrame;

        std::vector<std::tuple<ResourceCopyData, uint64>> mScheduledWritingData;
        std::queue<std::tuple<ResourceCopyData, uint64>> mWritingsInProgress;
    };
}