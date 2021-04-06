#pragma once

#include <Scene/SceneForwards.h>
#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Render/Passes/Data/PassData.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::Passes
{
    struct ForwardPassData
    {
        CameraData camera;
        std::vector<MeshData> meshes;
        std::vector<LightData> lights;
        dx::XMFLOAT4X4 shadowTransform;
    };

    class ForwardPass : public RenderPassBaseWithData<ForwardPassData>
    {
    public:
        ForwardPass();
        ~ForwardPass() override;

        void PrepareResources(Render::ResourcePlanner* planner) override;

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) override;

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) override;

        void Render(Render::PassRenderContext& passContext) override;

    private:

        void Draw(const Scene::Mesh& node, const dx::XMMATRIX& world, Render::PassRenderContext& passContext);
    };

} // namespace Engine