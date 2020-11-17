#include "FrameTransientContext.h"

#include <Memory/UploadBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>

namespace Engine::Render
{
    void FrameTransientContext::Reset()
    {
        uploadBuffer->Reset();
        dynamicDescriptorHeap->Reset();
       // usingResources.clear();
    }
} // namespace Engine::Render