#include "Renderer.h"

#include <StringUtils.h>

#include <Scene/CubeMap.h>
#include <Scene/Texture.h>
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

#include <Memory/UploadBuffer.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>

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
            mFrameContexts[i].uploadBuffer = MakeShared<Memory::UploadBuffer>(mRenderContext->Device().Get(), 500 * 1024 * 1024);
            mFrameContexts[i].dynamicDescriptorHeap = MakeShared<Memory::DynamicDescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorSize);
        }

        mFrameResourceProvider = MakeUnique<FrameResourceProvider>(mRenderContext->Device(), mRenderContext->GetGlobalResourceStateTracker().get());
    }

    void Renderer::Deinitialize()
    {
    }

    void Renderer::RegisterRenderPass(UniquePtr<RenderPassBase> renderPass)
    {
        mRenderPasses.push_back(std::move(renderPass));
    }

    void Renderer::Render(Scene::SceneObject* scene, const Timer& timer)
    {
        auto currentBackbufferIndex = mRenderContext->GetCurrentBackBufferIndex();
        mFrameContexts[currentBackbufferIndex].Reset();

        UploadResources(scene, mRenderContext, mFrameContexts[currentBackbufferIndex].uploadBuffer);

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
                creationInfo.description.Width = mRenderContext->GetSwapChain()->GetWidth();
                creationInfo.description.Height = mRenderContext->GetSwapChain()->GetHeight();

                mFrameResourceProvider->CreateResource(resource.name, resource.creationInfo);
            }

            pass->CreateRootSignatures(mRenderContext->GetRootSignatureProvider());
            pass->CreatePipelineStates(mRenderContext->GetPipelineStateProvider());
        }
    }

    void Renderer::RenderPasses(Scene::SceneObject* scene, const Timer& timer)
    {
        for (auto& pass : mRenderPasses)
        {
            RenderPass(pass.get(), scene, timer);
        }
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
        passContext.scene = scene;
        passContext.frameResourceProvider = mFrameResourceProvider.get();
        passContext.timer = &timer;
        passContext.resourceStateTracker = MakeShared<ResourceStateTracker>(mRenderContext->GetGlobalResourceStateTracker());

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


    /// TODO: find better solution for uploading scene resources to GPU memory
    /// Let's stay this logic here for now.
    void Renderer::UploadResources(Scene::SceneObject *scene, SharedPtr<RenderContext> renderContext, SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        const auto &meshView = scene->GetRegistry().view<Scene::Components::MeshComponent>();
        const auto &cubeMapView = scene->GetRegistry().view<Scene::Components::CubeMapComponent>();

        auto stateTracker = MakeShared<ResourceStateTracker>(renderContext->GetGlobalResourceStateTracker());

        auto commandList = renderContext->CreateCopyCommandList();

        commandList->SetName(L"Uploading resources List");

        uploadBuffer->Reset();
        bool anythingToLoad = false;

        for (auto &&[entity, meshComponent] : meshView.proxy())
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

        for (auto &&[entity, cubeComponent] : cubeMapView.proxy())
        {
            if (!cubeComponent.cubeMap.vertexBuffer->GetD3D12Resource())
            {
                anythingToLoad = true;
                CommandListUtils::UploadVertexBuffer(renderContext, commandList, stateTracker, *cubeComponent.cubeMap.vertexBuffer, uploadBuffer);
            }
            if (!cubeComponent.cubeMap.indexBuffer->GetD3D12Resource())
            {
                anythingToLoad = true;
                CommandListUtils::UploadIndexBuffer(renderContext, commandList, stateTracker, *cubeComponent.cubeMap.indexBuffer, uploadBuffer);
            }
            if (!cubeComponent.cubeMap.texture->GetD3D12Resource())
            {
                anythingToLoad = true;
                CommandListUtils::UploadTexture(renderContext, commandList, stateTracker, cubeComponent.cubeMap.texture.get(), uploadBuffer);
            }
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