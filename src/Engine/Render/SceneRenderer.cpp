#include "SceneRenderer.h"

#include <Render/CommandQueue.h>

namespace Engine::Render
{
    void FrameTransientContext::Reset()
    {
        uploadBuffer->Reset();
        dynamicDescriptorHeap->Reset();
        usingResources.clear();
    }

    SceneRenderer::SceneRenderer(SharedPtr<RenderContext> renderContext) : mRenderContext(renderContext)
    {

    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::Initialize()
    {
        mGame = MakeUnique<Game>();
        mGame->Initialize(mRenderContext);

        auto cbvSrvUavDescriptorSize = mRenderContext->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (Size i = 0; i < std::size(mFrameContexts); ++i)
        {
            mFrameContexts[i].uploadBuffer = MakeShared<UploadBuffer>(mRenderContext->Device().Get(), 500 * 1024 * 1024);
            mFrameContexts[i].dynamicDescriptorHeap = MakeShared<DynamicDescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorSize);
        }
    }

    void SceneRenderer::Deinitialize()
    {
        mGame->Deinitialize();
    }

    void SceneRenderer::Render(Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();
        mFrameContexts[currentBackbufferIndex].Reset();

        mGame->UploadResources(scene, mRenderContext);

        auto commandList = mRenderContext->CreateGraphicsCommandList();

        PassContext passContext = {};

        passContext.frameContext = &mFrameContexts[currentBackbufferIndex];
        passContext.commandList = commandList;
        passContext.renderContext = mRenderContext;
        passContext.scene = scene;
        passContext.timer = &timer;
        passContext.resourceStateTracker = MakeShared<ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

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