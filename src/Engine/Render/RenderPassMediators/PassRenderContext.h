#pragma once

#include <Types.h>
#include <Render/RenderForwards.h>

#include <Scene/SceneForwards.h>
#include <Timer.h>

namespace Engine::Render
{
    class PassRenderContext
    {
    public:
        SharedPtr<Memory::ResourceStateTracker> resourceStateTracker;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        SharedPtr<RenderContext> renderContext;
        SharedPtr<PassCommandRecorder> commandRecorder;

        const FrameResourceProvider* frameResourceProvider;

        FrameTransientContext  * frameContext;

        const Timer * timer;
    };
} // namespace Engine::Render
