#pragma once

#include <Types.h>
#include <EngineConfig.h>
#include <Timer.h>

#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/UploadBuffer.h>
#include <Render/ResourceStateTracker.h>
#include <Scene/SceneForwards.h>
#include <Render/RenderContext.h>

#include <Render/FrameResourceProvider.h>
#include <Render/PassContext.h>
#include <Render/FrameTransientContext.h>
#include <Render/RenderPassBase.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{
    class Renderer
    {
    public:
        Renderer(SharedPtr<RenderContext> renderContext);
        ~Renderer();

    public:
        void Initialize();
        void Deinitialize();

        void Render(Scene::SceneObject* scene, const Timer& timer);

    private:
        void UploadResources(Scene::SceneObject *scene, SharedPtr<RenderContext> renderContext, SharedPtr<UploadBuffer> uploadBuffer);
    private:
        FrameTransientContext mFrameContexts[EngineConfig::SwapChainBufferCount];

    private:
        SharedPtr<RenderContext> mRenderContext;
        UniquePtr<RenderPassBase> mGame;
        UniquePtr<FrameResourceProvider> mFrameResourceProvider;
    };
} // namespace Engine::Render