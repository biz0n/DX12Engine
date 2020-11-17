#pragma once 

#include <Types.h>
#include <Memory/MemoryForwards.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{    
    class FrameTransientContext
    {
    public:
        SharedPtr<Memory::DynamicDescriptorHeap> dynamicDescriptorHeap;
        SharedPtr<Memory::UploadBuffer> uploadBuffer;
        //std::vector<ComPtr<ID3D12Resource>> usingResources;

        void Reset();
    };
    
} // namespace Engine::Render
