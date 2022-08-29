#pragma once

#include <Scene/SceneForwards.h>
#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Scene/Components/MeshComponent.h>
#include <Render/Shaders/ShaderTypes.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::Passes
{
    class ForwardPass : public RenderPassBase
    {
    public:
        ForwardPass();
        ~ForwardPass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(const RenderRequest& renderRequest, Render::PassRenderContext& passContext, const Timer& timer) override;

    private:

        void Draw(const RenderRequest& renderRequest, Index meshIndex, Render::PassRenderContext& passContext);
    };

} // namespace Engine