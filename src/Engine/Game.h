#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Render/PipelineStateProvider.h>
#include <Render/ShaderProvider.h>

#include <Timer.h>
#include <Events.h>

#include <bitset>
#include <DirectXMath.h>
#include <SwapChain.h>

#include <entt/fwd.hpp>

namespace Engine
{
    class CommandListContext;
    
    class App;
    class SwapChain;
    class RenderContext;
    class Keyboard;

    class Game
    {
    public:
        Game(SharedPtr<RenderContext> renderContext);
        ~Game();

        bool Initialize();


        void UploadResources(entt::registry* registry);
        void Draw(const Timer &time, entt::registry* registry);

        void Deinitialize();

        void Resize(int32 width, int32 height);

    private:

        void ResizeDepthBuffer(int32 width, int32 height);

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

        D3D12_VIEWPORT mScreenViewport;
        D3D12_RECT mScissorRect;

        SharedPtr<SwapChain> mCanvas;
        SharedPtr<RenderContext> mRenderContext;
        UniquePtr<Render::PipelineStateProvider> mPipelineStateProvider;
        UniquePtr<Render::ShaderProvider> mShaderProvider;
    };

} // namespace Engine