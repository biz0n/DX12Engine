#pragma once

#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Render/Passes/Data/PassData.h>

namespace Engine::Render::Passes
{
    struct CubePassData
    {
        CameraData camera;
        CubeData cube;
        bool hasCube;
    };

    class CubePass : public RenderPassBaseWithData<CubePassData>
    {
    public:
        CubePass();
        ~CubePass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassRenderContext& passContext) override;
    }; 
} // namespace Engine::Render::Passes
