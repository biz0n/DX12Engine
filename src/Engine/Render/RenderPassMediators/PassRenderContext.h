#pragma once

#include <Types.h>
#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>
#include <Timer.h>

namespace Engine::Render
{
    struct PassRenderContext
    {
        SharedPtr<Memory::ResourceStateTracker> resourceStateTracker;
        SharedPtr<PassCommandRecorder> commandRecorder;
        SharedPtr<const FrameResourceProvider> frameResourceProvider;
        SharedPtr<Memory::UploadBuffer> uploadBuffer;
    };
} // namespace Engine::Render
