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

    class Game
    {
    public:
        Game();
        ~Game();

        void UploadResources(Scene::SceneObject* scene, SharedPtr<RenderContext> renderContext, SharedPtr<UploadBuffer> uploadBuffer);

        void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider);

        void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider);

        void Draw(Render::PassContext& passContext);

    private:

        void CreateDepthBuffer(int32 width, int32 height, SharedPtr<RenderContext> renderContext);

        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh& node, const dx::XMMATRIX& world, Render::PassContext& passContext);

    private:
        ComPtr<ID3D12Resource> mDepthBuffer;
        DescriptorAllocation mDepthBufferDescriptor;
    };

} // namespace Engine