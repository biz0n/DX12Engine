#include "FrameTransientContext.h"

#include <Memory/UploadBuffer.h>

namespace Engine::Render
{
    void FrameTransientContext::Reset()
    {
        uploadBuffer->Reset();
       // usingResources.clear();
    }
} // namespace Engine::Render