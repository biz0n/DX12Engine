#include "Game.h"

#include <stdlib.h>
#include <ShaderTypes.h>

#include <Render/SwapChain.h>
#include <Memory/UploadBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DescriptorAllocator.h>
#include <Render/ResourceStateTracker.h>
#include <Render/RenderContext.h>

#include <Render/CommandListUtils.h>
#include <Render/CommandQueue.h>

#include <Scene/SceneObject.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/Texture.h>
#include <Scene/Vertex.h>
#include <Scene/Camera.h>
#include <Scene/PunctualLight.h>

#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Render/MeshRenderer.h>

#include <Render/RootSignature.h>

#include <entt/entt.hpp>

#include <DirectXTex.h>
#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine
{
    Game::Game() = default;

    Game::~Game() = default;

    void Game::CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider)
    {
        Render::RootSignatureBuilder builder = {};
        builder
            .AddCBVParameter(0, 0, D3D12_SHADER_VISIBILITY_VERTEX)
            .AddCBVParameter(1, 0)
            .AddCBVParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVParameter(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(1, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(3, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        rootSignatureProvider->BuildRootSignature("ForwardRootSignature", builder);
    }

    void Game::CreatePipelineStates(Render::PipelineStateProvider *pipelineStateProvider)
    {
        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;

        Render::PipelineStateProxy pipelineStateCullModeBack = {
            .rootSignatureName = "ForwardRootSignature",
            .inputLayout = Scene::Vertex::GetInputLayout(),
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = "Resources\\Shaders\\Forward.hlsl",
            .pixelShaderName = "Resources\\Shaders\\Forward.hlsl",
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM
            },
            .rasterizer = rasterizer,
            .depthStencil = CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}};
        
        Render::PipelineStateProxy pipelineStateCullModeNone = pipelineStateCullModeBack;
        pipelineStateCullModeNone.rasterizer.CullMode = D3D12_CULL_MODE_NONE;

        pipelineStateProvider->CreatePipelineState("ForwardPipeline::CullModeBack", pipelineStateCullModeBack);
        pipelineStateProvider->CreatePipelineState("ForwardPipeline::CullModeNone", pipelineStateCullModeNone);
    }

    void Game::UploadResources(Scene::SceneObject *scene, SharedPtr<RenderContext> renderContext, SharedPtr<UploadBuffer> uploadBuffer)
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
                anythingToLoad = anythingToLoad || true;
                CommandListUtils::UploadVertexBuffer(renderContext, commandList, stateTracker, *mesh.vertexBuffer, uploadBuffer);
            }
            if (!mesh.indexBuffer->GetD3D12Resource())
            {
                anythingToLoad = anythingToLoad || true;
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

    void Game::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh &mesh, const dx::XMMATRIX &world, Render::PassContext &passContext)
    {
        auto renderContext = passContext.renderContext;

        auto cb = CommandListUtils::GetMeshUniform(world);
        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto dynamicDescriptorHeap = passContext.frameContext->dynamicDescriptorHeap;
        auto resourceStateTracker = passContext.resourceStateTracker;

        if (mesh.material->GetProperties().doubleSided)
        {
            commandList->SetPipelineState(renderContext->GetPipelineStateProvider()->GetPipelineState("ForwardPipeline::CullModeNone").Get());
        }
        else
        {
            commandList->SetPipelineState(renderContext->GetPipelineStateProvider()->GetPipelineState("ForwardPipeline::CullModeBack").Get());
        }

        commandList->IASetPrimitiveTopology(mesh.primitiveTopology);

        CommandListUtils::BindMaterial(
            renderContext,
            commandList,
            resourceStateTracker,
            passContext.frameContext->uploadBuffer,
            dynamicDescriptorHeap,
            mesh.material);
        CommandListUtils::BindVertexBuffer(commandList, resourceStateTracker, *mesh.vertexBuffer);
        CommandListUtils::BindIndexBuffer(commandList, resourceStateTracker, *mesh.indexBuffer);

        dynamicDescriptorHeap->CommitStagedDescriptors(renderContext->Device(), commandList);

        commandList->DrawIndexedInstanced(static_cast<uint32>(mesh.indexBuffer->GetElementsCount()), 1, 0, 0, 0);
    }

    void Game::Draw(Render::PassContext &passContext)
    {
        auto renderContext = passContext.renderContext;
        auto canvas = renderContext->GetSwapChain();

        auto commandList = passContext.commandList;
        commandList->SetName(L"Render scene List");

        renderContext->GetEventTracker().StartGPUEvent("Render scene list", commandList);

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto width = canvas->GetWidth();
        auto height = canvas->GetHeight();

        
        CreateDepthBuffer(width, height, renderContext);
        passContext.frameContext->usingResources.push_back(mDepthBuffer);

        auto backBuffer = canvas->GetCurrentBackBuffer();

        auto rtv = canvas->GetCurrentRenderTargetView();

        CommandListUtils::TransitionBarrier(resourceStateTracker, canvas->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        auto screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        auto scissorRect = CD3DX12_RECT(0, 0, width, height);

        commandList->RSSetViewports(1, &screenViewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        FLOAT clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};
        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
        commandList->ClearDepthStencilView(mDepthBufferDescriptor.GetDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtv, false, &mDepthBufferDescriptor.GetDescriptor());

        auto rootSignature = renderContext->GetRootSignatureProvider()->GetRootSignature("ForwardRootSignature");
        commandList->SetGraphicsRootSignature(rootSignature->GetD3D12RootSignature().Get());

        passContext.frameContext->dynamicDescriptorHeap->ParseRootSignature(rootSignature);

        float aspectRatio = canvas->GetWidth() / static_cast<float>(canvas->GetHeight());

        auto &registry = passContext.scene->GetRegistry();
        
        const auto &lightsView = registry.view<Scene::Components::LightComponent, Scene::Components::WorldTransformComponent>();
        std::vector<LightUniform> lights;
        lights.reserve(lightsView.size());
        for (auto &&[entity, lightComponent, transformComponent] : lightsView.proxy())
        {
            LightUniform light = CommandListUtils::GetLightUniform(lightComponent.light, transformComponent.transform);

            lights.emplace_back(light);
        }

        auto cameraEntity = registry.view<Scene::Components::CameraComponent, Scene::Components::WorldTransformComponent>().front();
        auto [camera, cameraWT] = registry.get<Scene::Components::CameraComponent, Scene::Components::WorldTransformComponent>(cameraEntity);
        auto cb = CommandListUtils::GetFrameUniform(camera.camera, cameraWT.transform, aspectRatio, static_cast<uint32>(lights.size()));

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(1, cbAllocation.GPU);

        auto lightsAllocation = passContext.frameContext->uploadBuffer->Allocate(lights.size() * sizeof(LightUniform), sizeof(LightUniform));

        lightsAllocation.CopyTo(lights);

        commandList->SetGraphicsRootShaderResourceView(3, lightsAllocation.GPU);


        const auto &meshsView = registry.view<Scene::Components::MeshComponent, Scene::Components::WorldTransformComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);
        for (auto &&[entity, meshComponent, transformComponent] : meshsView.proxy())
        {
            Draw(commandList, meshComponent.mesh, transformComponent.transform, passContext);
        }

        renderContext->GetEventTracker().EndGPUEvent(commandList);
    }

    void Game::CreateDepthBuffer(int32 width, int32 height, SharedPtr<RenderContext> renderContext)
    {
        if (mDepthBuffer != nullptr && mDepthBuffer->GetDesc().Width == width && mDepthBuffer->GetDesc().Height == height)
        {
            return;
        }

        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        ThrowIfFailed(renderContext->Device()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&mDepthBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        mDepthBufferDescriptor = renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->Allocate();
        renderContext->Device()->CreateDepthStencilView(
            mDepthBuffer.Get(),
            &dsv,
            mDepthBufferDescriptor.GetDescriptor());

        mDepthBuffer->SetName(L"Depth buffer");
    }

} // namespace Engine