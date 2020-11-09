#pragma once

#include <Types.h>
#include <EngineConfig.h>
#include <Timer.h>

#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/UploadBuffer.h>
#include <Render/ResourceStateTracker.h>
#include <Scene/SceneForwards.h>
#include <Render/RenderContext.h>
#include <Render/Game.h>

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

        void Reset();
    };

    class PassContext
    {
    public:
        SharedPtr<ResourceStateTracker> resourceStateTracker;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        SharedPtr<RenderContext> renderContext;

        FrameTransientContext  * frameContext;

        Scene::SceneObject * scene;

        const Timer * timer;
    };

    class MeshRenderer
    {
    public:
        MeshRenderer(SharedPtr<RenderContext> renderContext);
        ~MeshRenderer();

    public:
        void Initialize();
        void Deinitialize();

        void Render(Scene::SceneObject* scene, const Timer& timer);
    private:
        FrameTransientContext mFrameContexts[EngineConfig::SwapChainBufferCount];

    private:
        SharedPtr<RenderContext> mRenderContext;
        UniquePtr<Game> mGame;
    };
} // namespace Engine::Render