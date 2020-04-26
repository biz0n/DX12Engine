#pragma once

#include "IGame.h"
#include "UploadBuffer.h"
#include "SceneLoader.h"
#include "Camera.h"
#include "Types.h"
#include "RootSignature.h"
#include "DynamicDescriptorHeap.h"
#include "DescriptorAllocator.h"
#include "ResourceStateTracker.h"
#include "ImGuiManager.h"


#include <bitset>

class CommandListContext;

class Game : public IGame
{
public:
    Game(App *app);
    virtual ~Game();

    virtual bool Initialize();

    virtual void Update(const Timer &time);

    virtual void Draw(const Timer &time);

    virtual void Deinitialize();

    virtual void Resize(int32 width, int32 height);

    virtual void KeyPressed(KeyEvent event);

private:
    void UpdateBufferResoure(
        ComPtr<ID3D12GraphicsCommandList>,
        ID3D12Resource **,
        ID3D12Resource **,
        Size, Size,
        const void *,
        D3D12_RESOURCE_FLAGS);

    void ResizeDepthBuffer(int32 width, int32 height);

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, Node* node, UploadBuffer* buffer);
    void UploadMeshes(ComPtr<ID3D12GraphicsCommandList> commandList, Node* node, CommandListContext& commandListContext);

    DescriptorAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 count = 1)
    {
        return mDescriptorAllocators[heapType]->Allocate(Graphics().GetDevice(), count);
    }
private:
    uint64 mFenceValues[App::SwapChainBufferCount] = {0,0};

    SharedPtr<DynamicDescriptHeap> mDynamicDescriptorHeaps[App::SwapChainBufferCount];
    UniquePtr<DescriptorAllocator> mDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

private:
    UniquePtr<UploadBuffer> mUploadBuffer[App::SwapChainBufferCount];

    SharedPtr<GlobalResourceStateTracker> mGlobalResourceStateTracker;
    SharedPtr<ResourceStateTracker> mResourceStateTrackers[App::SwapChainBufferCount];

    UniquePtr<ImGuiManager> mImGuiManager;

private:
    UniquePtr<RootSignature> mRootSignature;
    ComPtr<ID3D12PipelineState> mPipelineState;

    ComPtr<ID3D12Resource> mDepthBuffer;
    DescriptorAllocation mDepthBufferDescriptor;

    D3D12_VIEWPORT mScreenViewport;
    D3D12_RECT mScissorRect;

    XMMATRIX mModelMatrix;
    XMMATRIX mViewMatrix;
    XMMATRIX mProjectionMatrix;

    UniquePtr<SceneObject> loadedScene;

    Camera mCamera{{0.0f, 2.0f, 0.0f}, 0.0f, Math::PI / 2};

    std::bitset<0xFF> keyState;
};