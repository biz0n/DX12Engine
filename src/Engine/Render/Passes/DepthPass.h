#pragma once

#include <Scene/SceneForwards.h>
#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Render/Passes/Data/PassData.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::Passes
{
    struct DepthPassData
    {
        CameraData camera;
        std::vector<MeshData> meshes;
    };

    class DepthPass : public RenderPassBaseWithData<DepthPassData>
    {
    public:
        DepthPass();
        ~DepthPass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassRenderContext& passContext) override;

    private:
        void Draw(const Scene::Mesh& node, const dx::XMMATRIX& world, Render::PassRenderContext& passContext);
    };

} // namespace Engine