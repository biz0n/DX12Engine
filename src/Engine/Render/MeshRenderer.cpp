#include "MeshRenderer.h"

#include <Render/CommandQueue.h>

#include <Render/CommandListUtils.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/SceneObject.h>
#include <Render/ResourcePlanner.h>

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

        mFrameResourceProvider = MakeUnique<FrameResourceProvider>(mRenderContext->Device(), mRenderContext->GetGlobalResourceStateTracker().get());
    }

    void MeshRenderer::Deinitialize()
    {
    }

    void MeshRenderer::Render(Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();
        mFrameContexts[currentBackbufferIndex].Reset();

        UploadResources(scene, mRenderContext, mFrameContexts[currentBackbufferIndex].uploadBuffer);

        auto commandList = mRenderContext->CreateGraphicsCommandList();

        PassContext passContext = {};

        passContext.frameContext = &mFrameContexts[currentBackbufferIndex];
        passContext.commandList = commandList;
        passContext.renderContext = mRenderContext;
        passContext.scene = scene;
        passContext.frameResourceProvider = mFrameResourceProvider.get();
        passContext.timer = &timer;
        passContext.resourceStateTracker = MakeShared<ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

        ResourcePlanner planner;

        mGame->PrepareResources(&planner);

        for (auto resource : planner.GetPlannedResources())
        {
            auto& creationInfo = resource.creationInfo;
            creationInfo.description.Width = mRenderContext->GetSwapChain()->GetWidth();
            creationInfo.description.Height = mRenderContext->GetSwapChain()->GetHeight();

            mFrameResourceProvider->CreateResource(resource.name, resource.creationInfo);
        }

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


    /// TODO: find better solution for uploading scene resources to GPU memory
    /// Let's stay this logic here for now.
    void MeshRenderer::UploadResources(Scene::SceneObject *scene, SharedPtr<RenderContext> renderContext, SharedPtr<UploadBuffer> uploadBuffer)
    {
        const auto &view = scene->GetRegistry().view<Scene::Components::MeshComponent>();

        auto stateTracker = MakeShared<ResourceStateTracker>(renderContext->GetGlobalResourceStateTracker());

        auto commandList = renderContext->CreateCopyCommandList();

        commandList->SetName(L"Uploading resources List");

        uploadBuffer->Reset();
        bool anythingToLoad = false;

        for (auto &&[entity, meshComponent] : view.proxy())
        {
            auto mesh = meshComponent.mesh;
            if (!mesh.vertexBuffer->GetD3D12Resource())
            {
                anythingToLoad = true;
                CommandListUtils::UploadVertexBuffer(renderContext, commandList, stateTracker, *mesh.vertexBuffer, uploadBuffer);
            }
            if (!mesh.indexBuffer->GetD3D12Resource())
            {
                anythingToLoad = true;
                CommandListUtils::UploadIndexBuffer(renderContext, commandList, stateTracker, *mesh.indexBuffer, uploadBuffer);
            }
            anythingToLoad = CommandListUtils::UploadMaterialTextures(renderContext, commandList, stateTracker, mesh.material, uploadBuffer) || anythingToLoad;
        }

        std::vector<ID3D12CommandList *> commandLists;

        auto barriersCommandList = renderContext->CreateCopyCommandList();

        auto barriers = stateTracker->FlushPendingBarriers(barriersCommandList);
        stateTracker->CommitFinalResourceStates();

        barriersCommandList->Close();
        commandList->Close();

        if (barriers > 0)
        {
            commandLists.push_back(barriersCommandList.Get());
        }

        if (anythingToLoad)
        {
            commandLists.push_back(commandList.Get());
        }

        if (commandLists.size() > 0)
        {
            uint64 fenceValue = renderContext->GetCopyCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());

            renderContext->GetGraphicsCommandQueue()->InsertWaitForQueue(renderContext->GetCopyCommandQueue(), fenceValue);
        }
    }
}