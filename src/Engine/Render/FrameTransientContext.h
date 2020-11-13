#pragma once 

#include <Types.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/UploadBuffer.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{    
    class FrameTransientContext
    {
    public:
        SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap;
        SharedPtr<UploadBuffer> uploadBuffer;
        std::vector<ComPtr<ID3D12Resource>> usingResources;

        void Reset()
        {
            uploadBuffer->Reset();
            dynamicDescriptorHeap->Reset();
            usingResources.clear();
        }
    };
    
} // namespace Engine::Render
