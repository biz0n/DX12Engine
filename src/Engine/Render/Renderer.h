#pragma once

#include <Types.h>
#include <EngineConfig.h>
#include <Timer.h>

#include <Memory/MemoryForwards.h>
#include <Render/RenderForwards.h>
#include <Scene/SceneForwards.h>

#include <Render/FrameTransientContext.h>


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

        void RegisterRenderPass(RenderPassBase* renderPass);

    private:
        void PrepareFrame();
        void RenderPasses(Scene::SceneObject* scene, const Timer& timer);
        void RenderPass(RenderPassBase* pass, Scene::SceneObject* scene, const Timer& timer);
        void UploadResources(SharedPtr<RenderContext> renderContext);
    private:
        FrameTransientContext mFrameContexts[EngineConfig::SwapChainBufferCount];

    private:
        SharedPtr<RenderContext> mRenderContext;
        std::vector<RenderPassBase*> mRenderPasses;
        UniquePtr<FrameResourceProvider> mFrameResourceProvider;
    };
} // namespace Engine::Render