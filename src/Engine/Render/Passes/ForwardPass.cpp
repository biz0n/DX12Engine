#include "ForwardPass.h"

#include <stdlib.h>
#include <ShaderTypes.h>

#include <Render/Passes/Names.h>

#include <Scene/SceneObject.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/Camera.h>
#include <Scene/PunctualLight.h>
#include <Scene/Vertex.h>

#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/IsDisabledComponent.h>

#include <Render/SwapChain.h>
#include <Render/ResourceStateTracker.h>
#include <Render/CommandQueue.h>
#include <Render/RootSignatureBuilder.h>
#include <Render/CommandListUtils.h>
#include <Render/Texture.h>
#include <Render/TextureCreationInfo.h>
#include <Render/PassContext.h>
#include <Render/PipelineStateStream.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/FrameTransientContext.h>
#include <Render/ShaderProvider.h>
#include <Render/PipelineStateProvider.h>
#include <Render/RootSignatureProvider.h>
#include <Render/ResourcePlanner.h>

#include <Memory/UploadBuffer.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/IndexBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>

#include <entt/entt.hpp>

#include <DirectXTex.h>
#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine::Render::Passes
{
    ForwardPass::ForwardPass() : Render::RenderPassBase("Forward Pass")
    {
    }

    ForwardPass::~ForwardPass() = default;

    void ForwardPass::CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider)
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

        rootSignatureProvider->BuildRootSignature(RootSignatureNames::Forward, builder);
    }

    void ForwardPass::CreatePipelineStates(Render::PipelineStateProvider *pipelineStateProvider)
    {
        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;

        Render::PipelineStateProxy pipelineStateCullModeBack = {
            .rootSignatureName = RootSignatureNames::Forward,
            .inputLayout = Scene::Vertex::GetInputLayout(),
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = Shaders::ForwardVS,
            .pixelShaderName = Shaders::ForwardPS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R16G16B16A16_FLOAT
            },
            .rasterizer = rasterizer,
            .depthStencil = CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}};
        
        Render::PipelineStateProxy pipelineStateCullModeNone = pipelineStateCullModeBack;
        pipelineStateCullModeNone.rasterizer.CullMode = D3D12_CULL_MODE_NONE;

        pipelineStateProvider->CreatePipelineState(PSONames::ForwardCullBack, pipelineStateCullModeBack);
        pipelineStateProvider->CreatePipelineState(PSONames::ForwardCullNone, pipelineStateCullModeNone);
    }

    void ForwardPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        Render::TextureCreationInfo dsTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, 0, 0, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            .clearValue = optimizedClearValue
        };
        planner->NewDepthStencil(ResourceNames::ForwardDepth, dsTexture);

        float clear[4] = {0};
        Render::TextureCreationInfo rtTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 0, 0, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
            .clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, clear)
        };
        planner->NewDepthStencil(ResourceNames::ForwardOutput, rtTexture);
    }

    void ForwardPass::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh &mesh, const dx::XMMATRIX &world, Render::PassContext &passContext)
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
            commandList->SetPipelineState(renderContext->GetPipelineStateProvider()->GetPipelineState(PSONames::ForwardCullNone).Get());
        }
        else
        {
            commandList->SetPipelineState(renderContext->GetPipelineStateProvider()->GetPipelineState(PSONames::ForwardCullBack).Get());
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

    void ForwardPass::Render(Render::PassContext &passContext)
    {
        auto renderContext = passContext.renderContext;
        auto canvas = renderContext->GetSwapChain();

        auto commandList = passContext.commandList;

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto width = canvas->GetWidth();
        auto height = canvas->GetHeight();

        Render::Texture* rtTexture = passContext.frameResourceProvider->GetTexture(ResourceNames::CubeOutput);
        Render::Texture* texture = passContext.frameResourceProvider->GetTexture(ResourceNames::ForwardDepth);
        CommandListUtils::TransitionBarrier(resourceStateTracker, texture->D3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

        passContext.frameContext->usingResources.push_back(texture->D3D12Resource());

        auto backBuffer = rtTexture->D3D12ResourceCom();// canvas->GetCurrentBackBuffer();

        auto rtv = rtTexture->GetRTDescriptor(renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).get());// canvas->GetCurrentRenderTargetView();

        CommandListUtils::TransitionBarrier(resourceStateTracker, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

        auto screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        auto scissorRect = CD3DX12_RECT(0, 0, width, height);

        commandList->RSSetViewports(1, &screenViewport);
        commandList->RSSetScissorRects(1, &scissorRect);

       // FLOAT clearColor[] = {0};
       // commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

        auto descriptor = texture->GetDSDescriptor(renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).get());
        commandList->ClearDepthStencilView(descriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtv, false, &descriptor);

        auto rootSignature = renderContext->GetRootSignatureProvider()->GetRootSignature(RootSignatureNames::Forward);
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
    }

} // namespace Engine