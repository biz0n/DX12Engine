#pragma once

#include <Render/RenderPassBase.h>
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
        virtual ~CubePass();

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassContext& passContext) override;
    }; 
} // namespace Engine::Render::Passes
