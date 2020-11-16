#pragma once

#include <Render/RenderPassBase.h>

namespace Engine::Render::Passes
{
    class CubePass : public RenderPassBase
    {
    public:
        CubePass();
        virtual ~CubePass();

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassContext& passContext) override;
    }; 
} // namespace Engine::Render::Passes
