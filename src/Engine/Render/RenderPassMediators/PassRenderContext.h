#pragma once

#include <Types.h>
#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>
#include <Timer.h>

namespace Engine::Render
{
    class PassRenderContext
    {
    public:
        SharedPtr<Memory::ResourceStateTracker> resourceStateTracker;
        //ComPtr<ID3D12GraphicsCommandList> commandList;
       // SharedPtr<RenderContext> renderContext;
        SharedPtr<PassCommandRecorder> commandRecorder;

        const FrameResourceProvider* frameResourceProvider;

        SharedPtr<Memory::UploadBuffer> uploadBuffer;

        SharedPtr<const Scene::SceneStorage> sceneStorage;

        const Timer * timer;
    };
} // namespace Engine::Render
