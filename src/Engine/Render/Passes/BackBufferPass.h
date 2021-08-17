#pragma once

#include <Render/RenderPassMediators/RenderPassBase.h>

namespace Engine::Render::Passes
{
    class BackBufferPass : public RenderPassBase
    {
    public:
        BackBufferPass();
        ~BackBufferPass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassRenderContext& passContext) override;
    }; 
} // namespace Engine::Render::Passes