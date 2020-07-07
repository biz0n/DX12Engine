#pragma once

#include <Scene/SceneForwards.h>
#include <Types.h>
#include <RootSignature.h>

#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <ResourceStateTracker.h>
#include <ImGuiManager.h>

#include <Timer.h>
#include <Events.h>

#include <bitset>
#include <DirectXMath.h>

namespace Engine
{
    class CommandListContext;
    
    class App;
    class Canvas;
    class RenderContext;

    class Game
    {
    public:
        Game(App *app, SharedPtr<RenderContext> renderContext, SharedPtr<Canvas> canvas);
        ~Game();

        bool Initialize();

        void Update(const Timer &time);

        void Draw(const Timer &time);

        void Deinitialize();

        void Resize(int32 width, int32 height);

        void KeyPressed(KeyEvent event);

    private:
        static const uint32 SwapChainBufferCount = 2;

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
        uint64 mFenceValues[SwapChainBufferCount] = {0, 0};

        SharedPtr<DynamicDescriptorHeap> mDynamicDescriptorHeaps[SwapChainBufferCount];

    private:
        SharedPtr<UploadBuffer> mUploadBuffer[SwapChainBufferCount];

        SharedPtr<ResourceStateTracker> mResourceStateTrackers[SwapChainBufferCount];

        UniquePtr<ImGuiManager> mImGuiManager;

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

        SharedPtr<Canvas> mCanvas;
        SharedPtr<RenderContext> mRenderContext;

        std::bitset<0xFF> keyState;

        App &Graphics() { return *mApp; }
        App *mApp;
    };

} // namespace Engine