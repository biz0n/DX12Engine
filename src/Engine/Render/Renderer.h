#pragma once

#include <Types.h>
#include <EngineConfig.h>
#include <Timer.h>

#include <Memory/MemoryForwards.h>
#include <Render/RenderForwards.h>
#include <Graph/GraphForwards.h>
#include <Graph/GraphBuilder.h>
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

        void Render(const RenderRequest &renderRequest, const Timer& timer);
        void Reset();

        void RegisterRenderPass(RenderPassBase* renderPass);

        const Graph::GraphBuilder& GetGraph() const { return mGraphBuilder; }

    private:
        void PrepareFrame();
        void RenderPasses(const RenderRequest& renderRequest, const Timer& timer);
        void RenderPass(PassContext* passContext, const RenderRequest& renderRequest, const Timer& timer);
        void UploadResources(SharedPtr<RenderContext> renderContext);

    private:
        SharedPtr<RenderContext> mRenderContext;
        std::vector<RenderPassBase*> mRenderPasses;
        std::vector<PassContext> mPassContexts;
        SharedPtr<FrameResourceProvider> mFrameResourceProvider;
        Graph::GraphBuilder mGraphBuilder;
    };
} // namespace Engine::Render