#include "RenderContext.h"

#include <Exceptions.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine
{
    RenderContext::RenderContext()
    {
        mGraphics = MakeUnique<Graphics>();

        for (uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
            uint32 incrementalSize = Device()->GetDescriptorHandleIncrementSize(type);
            mDescriptorAllocators[i] = MakeUnique<DescriptorAllocator>(Device(), type);
        }

        mDirrectCommandQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        mComputeCommandQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
        mCopyCommandQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

        mGlobalResourceStateTracker = MakeShared<GlobalResourceStateTracker>();
    }

    ComPtr<ID3D12Device2> RenderContext::Device() const
    {
        return mGraphics->GetDevice();
    }

    ComPtr<IDXGIFactory4> RenderContext::GIFactory() const
    {
        return mGraphics->GetGIFactory();
    }

    DescriptorAllocation RenderContext::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 count)
    {
        return mDescriptorAllocators[heapType]->Allocate(count);    
    }

    SharedPtr<GlobalResourceStateTracker> RenderContext::GetResourceStateTracker() const
    {
        return mGlobalResourceStateTracker;
    }

    ComPtr<ID3D12CommandQueue> RenderContext::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
    {
        switch (type)
        {
            case D3D12_COMMAND_LIST_TYPE_DIRECT:
                return mDirrectCommandQueue;
            case D3D12_COMMAND_LIST_TYPE_COMPUTE:
                return mComputeCommandQueue;
            case D3D12_COMMAND_LIST_TYPE_COPY:
                return mCopyCommandQueue;
        }

        assert(false && "Invalid command queue type.");
        return nullptr;
        
    }

    ComPtr<ID3D12CommandQueue> RenderContext::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandQueue> commandQueue;

        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        ThrowIfFailed(Device()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

        return commandQueue;
    }
}