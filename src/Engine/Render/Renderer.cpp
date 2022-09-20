#include "Renderer.h"

#include <HAL/CommandQueue.h>
#include <HAL/SwapChain.h>
#include <HAL/DirectXExtensions.h>

#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>
#include <Render/RenderPassMediators/PassContext.h>

#include <Render/FrameResourceProvider.h>
#include <Render/RenderContext.h>

#include <Graph/Node.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/TextureCreationInfo.h>
#include <Memory/Texture.h>
#include <Memory/UploadBuffer.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/ResourceFactory.h>
#include <Memory/ResourceCopyManager.h>

#include <d3d12.h>

namespace Engine::Render
{
    Renderer::Renderer(SharedPtr<RenderContext> renderContext) : mRenderContext(renderContext)
    {
        mFrameResourceProvider = MakeShared<FrameResourceProvider>(mRenderContext->Device(), mRenderContext->GetSwapChain(), mRenderContext->GetResourceFactory());
    }

    Renderer::~Renderer() = default;

    void Renderer::Initialize()
    {
        
    }

    void Renderer::Deinitialize()
    {
    }

    void Renderer::RegisterRenderPass(RenderPassBase* renderPass)
    {
        mGraphBuilder.AddNode(Graph::Node{ Name(renderPass->GetName()), (int)renderPass->GetQueueType() });
        mRenderPasses.push_back(renderPass);
    }

    void Renderer::Render(const RenderRequest& renderRequest, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();

        UploadResources(mRenderContext);

        PrepareFrame();

        RenderPasses(renderRequest, timer);
    }

    void Renderer::PrepareFrame()
    {
        mPassContexts.reserve(mRenderPasses.size());
        mFrameResourceProvider->PrepareResources();
        for (Index i = 0; i < mRenderPasses.size(); ++i)
        {
            RenderPassBase* pass = mRenderPasses[i];
            Graph::Node* node = mGraphBuilder.GetNode(i);

            PassContext passContext(pass, node, mFrameResourceProvider);

            pass->PrepareResources(passContext.GetResourcePlanner());

            pass->CreateRootSignatures(mRenderContext->GetRootSignatureProvider());
            pass->CreatePipelineStates(mRenderContext->GetPipelineStateProvider());

            mPassContexts.push_back(std::move(passContext));
        }

        mFrameResourceProvider->CreateResources();
    }

    void Renderer::RenderPasses(const RenderRequest& renderRequest, const Timer& timer)
    {
        mGraphBuilder.Build();

        for (auto orderedIndex : mGraphBuilder.GetOrderedIndexes())
        {
            auto pass = &mPassContexts[orderedIndex];
            RenderPass(pass, renderRequest, timer);
        }

        mRenderPasses.clear();
        mPassContexts.clear();
    }

    void Renderer::Reset()
    {
        mGraphBuilder.Clear();
    }

    void Renderer::RenderPass(PassContext* passContext, const RenderRequest& renderRequest, const Timer& timer)
    {
        auto* pass = passContext->GetRenderPass();
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();

        bool isGraphicPass = passContext->GetQueueType() == CommandQueueType::Graphics;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        if (isGraphicPass)
        {
            commandList = mRenderContext->CreateGraphicsCommandList();
        }
        else
        {
            commandList = mRenderContext->CreateComputeCommandList();
        }

        HAL::SetResourceName(commandList, pass->GetName() + " CL");
        
        mRenderContext->GetEventTracker().StartGPUEvent(pass->GetName(), commandList);

        PassRenderContext passRenderContext = {};

        passRenderContext.uploadBuffer = mRenderContext->GetUploadBuffer();
        passRenderContext.frameResourceProvider = mFrameResourceProvider;
        passRenderContext.resourceStateTracker = MakeShared<Memory::ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

        passRenderContext.commandRecorder = MakeShared<PassCommandRecorder>(
            passContext,
            commandList,
            passRenderContext.resourceStateTracker.get(),
            mRenderContext.get(),
            mFrameResourceProvider.get());

        pass->Render(renderRequest, passRenderContext, timer);

        passRenderContext.resourceStateTracker->FlushBarriers(commandList);

        mRenderContext->GetEventTracker().EndGPUEvent(commandList);
        
        commandList->Close();

        auto prePassCommandList = mRenderContext->CreateGraphicsCommandList();
        HAL::SetResourceName(commandList, pass->GetName() + " PrePass CL");
        mRenderContext->GetEventTracker().StartGPUEvent("PrePass: " + pass->GetName(), prePassCommandList);

        auto barriers = passRenderContext.resourceStateTracker->FlushPendingBarriers(prePassCommandList);
        passRenderContext.resourceStateTracker->CommitFinalResourceStates();
        passRenderContext.resourceStateTracker->FlushBarriers(prePassCommandList);

        mRenderContext->GetEventTracker().EndGPUEvent(prePassCommandList);
        prePassCommandList->Close();

        std::vector<ID3D12CommandList*> graphicsCommandLists;
        std::vector<ID3D12CommandList*> computeCommandLists;

        if (barriers > 0)
        {
            graphicsCommandLists.push_back(prePassCommandList.Get());
        }
        
        if (isGraphicPass)
        {
            graphicsCommandLists.push_back(commandList.Get());
        }
        else
        {
            computeCommandLists.push_back(commandList.Get());
        }
        
        if (!graphicsCommandLists.empty())
        {
            mRenderContext->GetGraphicsCommandQueue()->ExecuteCommandLists(graphicsCommandLists.size(), graphicsCommandLists.data());
        }

        if (!computeCommandLists.empty())
        {
            mRenderContext->GetEventTracker().StartGPUEvent("Wait for graphics queue", mRenderContext->GetComputeCommandQueue()->D3D12CommandQueue());
            mRenderContext->GetComputeCommandQueue()->InsertWaitForQueue(mRenderContext->GetGraphicsCommandQueue());
            mRenderContext->GetEventTracker().EndGPUEvent(mRenderContext->GetComputeCommandQueue()->D3D12CommandQueue());
            
            mRenderContext->GetComputeCommandQueue()->ExecuteCommandLists(computeCommandLists.size(), computeCommandLists.data());
            
            mRenderContext->GetEventTracker().StartGPUEvent("Wait for compute queue", mRenderContext->GetGraphicsCommandQueue()->D3D12CommandQueue());
            mRenderContext->GetGraphicsCommandQueue()->InsertWaitForQueue(mRenderContext->GetComputeCommandQueue());
            mRenderContext->GetEventTracker().EndGPUEvent(mRenderContext->GetGraphicsCommandQueue()->D3D12CommandQueue());
        }
    } 


    void Renderer::UploadResources(SharedPtr<RenderContext> renderContext)
    {
        auto stateTracker = MakeUnique<Memory::ResourceStateTracker>(renderContext->GetGlobalResourceStateTracker());
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
            renderContext->GetComputeCommandQueue()->InsertWaitForQueue(renderContext->GetGraphicsCommandQueue(), fenceValue);
        }
    }
}