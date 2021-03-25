#include "Renderer.h"

#include <StringUtils.h>

#include <Scene/CubeMap.h>
#include <Scene/SceneObject.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/CubeMapComponent.h>


#include <Render/CommandQueue.h>
#include <Render/CommandListUtils.h>
#include <Render/SwapChain.h>
#include <Render/ResourceStateTracker.h>
#include <Render/TextureCreationInfo.h>
#include <Render/FrameResourceProvider.h>
#include <Render/ResourcePlanner.h>
#include <Render/PassContext.h>
#include <Render/RenderContext.h>
#include <Render/RenderPassBase.h>
#include <Render/PassCommandRecorder.h>

#include <Memory/Texture.h>
#include <Memory/UploadBuffer.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/ResourceFactory.h>
#include <Memory/ResourceCopyManager.h>

#include <entt/entt.hpp>
#include <d3d12.h>

namespace Engine::Render
{
    Renderer::Renderer(SharedPtr<RenderContext> renderContext) : mRenderContext(renderContext)
    {

    }

    Renderer::~Renderer() = default;

    void Renderer::Initialize()
    {
        auto cbvSrvUavDescriptorSize = mRenderContext->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (Size i = 0; i < std::size(mFrameContexts); ++i)
        {
            mFrameContexts[i].uploadBuffer = mRenderContext->GetResourceFactory()->CreateUploadBuffer(500 * 1024 * 1024);
            mFrameContexts[i].dynamicDescriptorHeap = MakeShared<Memory::DynamicDescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorSize);
        }

        mFrameResourceProvider = MakeUnique<FrameResourceProvider>(mRenderContext->Device(), mRenderContext->GetResourceFactory());
    }

    void Renderer::Deinitialize()
    {
    }

    void Renderer::RegisterRenderPass(RenderPassBase* renderPass)
    {
        mRenderPasses.push_back(renderPass);
    }

    void Renderer::Render(Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();
        mFrameContexts[currentBackbufferIndex].Reset();

        UploadResources(mRenderContext);

        PrepareFrame();

        RenderPasses(scene, timer);
    }

    void Renderer::PrepareFrame()
    {
        for (auto& pass : mRenderPasses)
        {
            ResourcePlanner planner;

            pass->PrepareResources(&planner);

            for (auto resource : planner.GetPlannedResources())
            {
                auto& creationInfo = resource.creationInfo;
                if (creationInfo.description.Width == 0 && creationInfo.description.Height == 0)
                {
                    creationInfo.description.Width = mRenderContext->GetSwapChain()->GetWidth();
                    creationInfo.description.Height = mRenderContext->GetSwapChain()->GetHeight();
                }

                mFrameResourceProvider->CreateResource(resource.name, resource.creationInfo, resource.state);
            }

            pass->CreateRootSignatures(mRenderContext->GetRootSignatureProvider());
            pass->CreatePipelineStates(mRenderContext->GetPipelineStateProvider());
        }
    }

    void Renderer::RenderPasses(Scene::SceneObject* scene, const Timer& timer)
    {
        for (auto& pass : mRenderPasses)
        {
            RenderPass(pass, scene, timer);
        }

        mRenderPasses.clear();
    }

    void Renderer::RenderPass(RenderPassBase* pass, Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();

        auto commandList = mRenderContext->CreateGraphicsCommandList();
        commandList->SetName(StringToWString(pass->GetName() + " CL").c_str());
        
        mRenderContext->GetEventTracker().StartGPUEvent(pass->GetName(), commandList);

        PassContext passContext = {};

        passContext.frameContext = &mFrameContexts[currentBackbufferIndex];
        passContext.commandList = commandList;
        passContext.renderContext = mRenderContext;
        passContext.frameResourceProvider = mFrameResourceProvider.get();
        passContext.timer = &timer;
        passContext.resourceStateTracker = MakeShared<ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

        passContext.commandRecorder = MakeShared<PassCommandRecorder>(
            commandList,
            passContext.resourceStateTracker.get(),
            mRenderContext.get(),
            mFrameResourceProvider.get(),
            &mFrameContexts[currentBackbufferIndex]);

        pass->Render(passContext);

        passContext.resourceStateTracker->FlushBarriers(commandList);

        mRenderContext->GetEventTracker().EndGPUEvent(commandList);
        
        commandList->Close();

        auto prePassCommandList = mRenderContext->CreateGraphicsCommandList();
        prePassCommandList->SetName(StringToWString(pass->GetName() + " PrePass CL").c_str());
        mRenderContext->GetEventTracker().StartGPUEvent("PrePass: " + pass->GetName(), prePassCommandList);

        auto barriers = passContext.resourceStateTracker->FlushPendingBarriers(prePassCommandList);
        passContext.resourceStateTracker->CommitFinalResourceStates();
        passContext.resourceStateTracker->FlushBarriers(prePassCommandList);

        mRenderContext->GetEventTracker().EndGPUEvent(prePassCommandList);
        prePassCommandList->Close();

        std::vector<ID3D12CommandList*> commandLists;

        if (barriers > 0)
        {
            commandLists.push_back(prePassCommandList.Get());
        }
        
        commandLists.push_back(commandList.Get());
        
        mRenderContext->GetGraphicsCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());
    }


    void Renderer::UploadResources(SharedPtr<RenderContext> renderContext)
    {
        auto stateTracker = MakeUnique<ResourceStateTracker>(renderContext->GetGlobalResourceStateTracker());
        auto commandList = renderContext->CreateGraphicsCommandList();
        commandList->SetName(L"Uploading resources List");

        auto copyManager = mRenderContext->GetResourceCopyManager();

        bool anythingToLoad = copyManager->Copy(commandList.Get(), stateTracker.get());
        stateTracker->FlushBarriers(commandList);


        std::vector<ID3D12CommandList *> commandLists;

        auto barriersCommandList = renderContext->CreateGraphicsCommandList();

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
            commandLists.emplace_back(commandList.Get());
        }

        if (!commandLists.empty())
        {
            uint64 fenceValue = renderContext->GetGraphicsCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());

            renderContext->GetGraphicsCommandQueue()->InsertWaitForQueue(renderContext->GetGraphicsCommandQueue(), fenceValue);
        }
    }
}