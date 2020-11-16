#pragma once

#include <Scene/SceneForwards.h>
#include <Render/RenderPassBase.h>

#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine
{
    namespace Render
    {
        class PassContext;
    }
    class RenderContext;
}

namespace Engine::Render::Passes
{
    class ForwardPass : public RenderPassBase
    {
    public:
        ForwardPass();
        virtual ~ForwardPass();

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassContext& passContext) override;

    private:

        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh& node, const dx::XMMATRIX& world, Render::PassContext& passContext);
    };

} // namespace Engine