#pragma once

#include <Render/RenderPassBase.h>

namespace Engine::Render::Passes
{
    class ToneMappingPass : public RenderPassBase
    {
    public:
        ToneMappingPass();
        ~ToneMappingPass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassContext& passContext) override;
    }; 
} // namespace Engine::Render::Passes
