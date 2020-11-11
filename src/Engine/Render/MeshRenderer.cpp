#include "MeshRenderer.h"

#include <Render/CommandQueue.h>

namespace Engine::Render
{
    void FrameTransientContext::Reset()
    {
        uploadBuffer->Reset();
        dynamicDescriptorHeap->Reset();
        usingResources.clear();
    }

    MeshRenderer::MeshRenderer(SharedPtr<RenderContext> renderContext) : mRenderContext(renderContext)
    {

    }

    MeshRenderer::~MeshRenderer() = default;

    void MeshRenderer::Initialize()
    {
        mGame = MakeUnique<Game>();

        auto cbvSrvUavDescriptorSize = mRenderContext->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (Size i = 0; i < std::size(mFrameContexts); ++i)
        {
            mFrameContexts[i].uploadBuffer = MakeShared<UploadBuffer>(mRenderContext->Device().Get(), 500 * 1024 * 1024);
            mFrameContexts[i].dynamicDescriptorHeap = MakeShared<DynamicDescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorSize);
        }
    }

    void MeshRenderer::Deinitialize()
    {
    }

    void MeshRenderer::Render(Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();
        mFrameContexts[currentBackbufferIndex].Reset();

        mGame->UploadResources(scene, mRenderContext, mFrameContexts[currentBackbufferIndex].uploadBuffer);

        auto commandList = mRenderContext->CreateGraphicsCommandList();

        PassContext passContext = {};

        passContext.frameContext = &mFrameContexts[currentBackbufferIndex];
        passContext.commandList = commandList;
        passContext.renderContext = mRenderContext;
        passContext.scene = scene;
        passContext.timer = &timer;
        passContext.resourceStateTracker = MakeShared<ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

        mGame->CreateRootSignatures(mRenderContext->GetRootSignatureProvider());
        mGame->CreatePipelineStates(mRenderContext->GetPipelineStateProvider());

        mGame->Draw(passContext);

        passContext.resourceStateTracker->FlushBarriers(commandList);
        commandList->Close();

        auto prePassCommandList = mRenderContext->CreateGraphicsCommandList();
        auto barriers = passContext.resourceStateTracker->FlushPendingBarriers(prePassCommandList);
        passContext.resourceStateTracker->CommitFinalResourceStates();
        prePassCommandList->Close();

        std::vector<ID3D12CommandList*> commandLists;

        if (barriers > 0)
        {
            commandLists.push_back(prePassCommandList.Get());
        }
        
        commandLists.push_back(commandList.Get());
        

        mRenderContext->GetGraphicsCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());
    }
}