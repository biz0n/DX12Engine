#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <EngineConfig.h>
#include <Render/RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Render/PipelineStateProvider.h>
#include <Render/ShaderProvider.h>

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

        bool Initialize(SharedPtr<RenderContext> renderContext);

        void UploadResources(Scene::SceneObject* scene, SharedPtr<RenderContext> renderContext);
        void Draw(Render::PassContext& passContext);

        void Deinitialize();

    private:

        void CreateDepthBuffer(int32 width, int32 height, ComPtr<ID3D12Device> device);

        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh& node, const dx::XMMATRIX& world, Render::PassContext& passContext);

        ComPtr<ID3D12PipelineState> CreatePipelineState(const Scene::Mesh& mesh, SharedPtr<RenderContext> renderContext);

    private:

        SharedPtr<UploadBuffer> _mUploadBuffer[EngineConfig::SwapChainBufferCount];

    private:
        UniquePtr<RootSignature> mRootSignature;

        ComPtr<ID3D12Resource> mDepthBuffer;
        DescriptorAllocation mDepthBufferDescriptor;
    };

} // namespace Engine