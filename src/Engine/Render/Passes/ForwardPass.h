#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <EngineConfig.h>
#include <Render/RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Render/PipelineStateProvider.h>
#include <Render/ShaderProvider.h>
#include <Render/PipelineStateProvider.h>
#include <Render/RootSignatureProvider.h>
#include <Render/ResourcePlanner.h>

#include <Render/RenderPassBase.h>

#include <Timer.h>

#include <entt/fwd.hpp>
#include <DirectXMath.h>

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