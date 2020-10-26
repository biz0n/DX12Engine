#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>

#include <Timer.h>
#include <Events.h>

#include <bitset>
#include <DirectXMath.h>
#include <SwapChain.h>

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
        Game(SharedPtr<RenderContext> renderContext, SharedPtr<Keyboard> keyboard);
        ~Game();

        bool Initialize();

        void Update(const Timer &time);

        void Draw(const Timer &time);

        void Deinitialize();

        void Resize(int32 width, int32 height);

    private:

        void UpdateBufferResoure(
            ComPtr<ID3D12GraphicsCommandList>,
            ID3D12Resource **,
            ID3D12Resource **,
            Size, Size,
            const void *,
            D3D12_RESOURCE_FLAGS);

        void ResizeDepthBuffer(int32 width, int32 height);

        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const SharedPtr<Scene::MeshNode>& node, SharedPtr<UploadBuffer> buffer);
        void UploadMeshes(ComPtr<ID3D12GraphicsCommandList> commandList, const SharedPtr<Scene::MeshNode>& node, CommandListContext &commandListContext);

        SharedPtr<Scene::CameraNode> Camera() const;

    private:

        SharedPtr<DynamicDescriptorHeap> mDynamicDescriptorHeaps[SwapChain::SwapChainBufferCount];

    private:
        SharedPtr<UploadBuffer> mUploadBuffer[SwapChain::SwapChainBufferCount];

    private:
        UniquePtr<RootSignature> mRootSignature;
        ComPtr<ID3D12PipelineState> mPipelineState;

        ComPtr<ID3D12Resource> mDepthBuffer;
        DescriptorAllocation mDepthBufferDescriptor;

        D3D12_VIEWPORT mScreenViewport;
        D3D12_RECT mScissorRect;

        DirectX::XMMATRIX mModelMatrix;
        DirectX::XMMATRIX mViewMatrix;
        DirectX::XMMATRIX mProjectionMatrix;

        UniquePtr<Scene::SceneObject> loadedScene;

        SharedPtr<SwapChain> mCanvas;
        SharedPtr<RenderContext> mRenderContext;
        SharedPtr<Keyboard> mKeyboard;
    };

} // namespace Engine