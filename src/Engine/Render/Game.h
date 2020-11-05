#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <Render/RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Render/PipelineStateProvider.h>
#include <Render/ShaderProvider.h>

#include <Timer.h>

#include <Render/SwapChain.h>

#include <entt/fwd.hpp>
#include <DirectXMath.h>

namespace Engine
{
    class SwapChain;
    class RenderContext;

    class Game
    {
    public:
        Game(SharedPtr<RenderContext> renderContext);
        ~Game();

        bool Initialize();

        void UploadResources(Scene::SceneObject* scene);
        void Draw(const Timer &time, Scene::SceneObject* scene);

        void Deinitialize();

    private:

        void CreateDepthBuffer(int32 width, int32 height);

        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh& node, const dx::XMMATRIX& world, SharedPtr<UploadBuffer> buffer);

        ComPtr<ID3D12PipelineState> CreatePipelineState(const Scene::Mesh& mesh);

    private:

        SharedPtr<DynamicDescriptorHeap> mDynamicDescriptorHeaps[SwapChain::SwapChainBufferCount];
        SharedPtr<UploadBuffer> mUploadBuffer[SwapChain::SwapChainBufferCount];
        std::vector<ComPtr<ID3D12Resource>> mFrameResources[SwapChain::SwapChainBufferCount];

    private:
        UniquePtr<RootSignature> mRootSignature;
        ComPtr<ID3D12PipelineState> mPipelineState;

        ComPtr<ID3D12Resource> mDepthBuffer;
        DescriptorAllocation mDepthBufferDescriptor;

        SharedPtr<SwapChain> mCanvas;
        SharedPtr<RenderContext> mRenderContext;
        UniquePtr<Render::PipelineStateProvider> mPipelineStateProvider;
        UniquePtr<Render::ShaderProvider> mShaderProvider;
    };

} // namespace Engine