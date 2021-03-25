//
// Created by Maxim on 3/24/2021.
//

#include "ResourceCopyManager.h"

#include <Render/ResourceStateTracker.h>

#include <d3dx12.h>

bool Engine::Memory::ResourceCopyManager::Copy(ID3D12GraphicsCommandList *commandList,
                                               Engine::Render::ResourceStateTracker *stateTracker)
{
    if (mScheduledWritingData.empty())
    {
        return false;
    }

    for (const auto& copyDataWithFrame : mScheduledWritingData)
    {
        mWritingsInProgress.push(copyDataWithFrame);
        const auto& copyData = std::get<ResourceCopyData>(copyDataWithFrame);
        stateTracker->ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(copyData.DestinationResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

    }

    for (const auto& copyDataWithFrame : mScheduledWritingData)
    {
        const auto& copyData = std::get<ResourceCopyData>(copyDataWithFrame);
        copyData.Execute(commandList);

        stateTracker->ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(copyData.DestinationResource, D3D12_RESOURCE_STATE_COMMON, copyData.StateAfterCopy));
    }

    mScheduledWritingData.clear();

    return true;
}
