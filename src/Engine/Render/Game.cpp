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

#include <Scene/Loader/SceneLoader.h>
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
#include <Render/SceneRenderer.h>

#include <Render/ShaderCreationInfo.h>

#include <Render/RootSignature.h>

#include <entt/entt.hpp>

#include <DirectXTex.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3d12.h>

namespace Engine
{
    Game::Game()
    {
    }

    Game::~Game()
    {
    }

    bool Game::Initialize(SharedPtr<RenderContext> renderContext)
    {
        for (uint32 frameIndex = 0; frameIndex < EngineConfig::SwapChainBufferCount; ++frameIndex)
        {
            _mUploadBuffer[frameIndex] = MakeShared<UploadBuffer>(renderContext->Device().Get(), 512 * 1024 * 1024);
        }

        mShaderProvider = MakeUnique<Render::ShaderProvider>();

        CD3DX12_DESCRIPTOR_RANGE1 texTable1;
        texTable1.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            0); // register t0

        CD3DX12_DESCRIPTOR_RANGE1 texTable2;
        texTable2.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            1); // register t1

        CD3DX12_DESCRIPTOR_RANGE1 texTable3;
        texTable3.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            2); // register t2

        CD3DX12_DESCRIPTOR_RANGE1 texTable4;
        texTable4.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            3); // register t2

        CD3DX12_DESCRIPTOR_RANGE1 texTable5;
        texTable5.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            4); // register t2

        CD3DX12_ROOT_PARAMETER1 rootParameters[9] = {};

        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

        rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[3].InitAsShaderResourceView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[6].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[7].InitAsDescriptorTable(1, &texTable4, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[8].InitAsDescriptorTable(1, &texTable5, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(
            0,                               // shaderRegister
            D3D12_FILTER_ANISOTROPIC,        // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
                             D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        mRootSignature = MakeUnique<RootSignature>(renderContext->Device(), &rootSigDesc);

        mPipelineStateProvider = MakeUnique<Render::PipelineStateProvider>(renderContext->Device());

        return true;
    }

    void Game::Deinitialize()
    {
    }

    ComPtr<ID3D12PipelineState> Game::CreatePipelineState(const Scene::Mesh& mesh, SharedPtr<RenderContext> renderContext)
    {
        auto inputLayout = Scene::Vertex::GetInputLayout();

        ComPtr<ID3DBlob> pixelShaderBlob = mShaderProvider->GetShader(Render::ShaderCreationInfo("Resources\\Shaders\\Forward.hlsl", "mainPS", "ps_5_1"));

        ComPtr<ID3DBlob> vertexShaderBlob = mShaderProvider->GetShader(Render::ShaderCreationInfo("Resources\\Shaders\\Forward.hlsl", "mainVS", "vs_5_1"));


        Render::PipelineStateStream pipelineStateStream;

        D3D12_RT_FORMAT_ARRAY rtvFormats = {};
        rtvFormats.NumRenderTargets = 1;
        rtvFormats.RTFormats[0] = renderContext->GetSwapChain()->GetCurrentBackBuffer()->GetDesc().Format;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;

        if (mesh.material->GetProperties().doubleSided)
        {
            rasterizer.CullMode = D3D12_CULL_MODE_NONE;
        }
        else
        {
            rasterizer.CullMode = D3D12_CULL_MODE_BACK;
        }

        pipelineStateStream.rootSignature = mRootSignature->GetD3D12RootSignature().Get();
        pipelineStateStream.inputLayout = {inputLayout.data(), static_cast<uint32>(inputLayout.size())};
        pipelineStateStream.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
        pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
        pipelineStateStream.dsvFormat = mDepthBuffer->GetDesc().Format;
        pipelineStateStream.rtvFormats = rtvFormats;
        pipelineStateStream.rasterizer = CD3DX12_RASTERIZER_DESC(rasterizer);

        return mPipelineStateProvider->CreatePipelineState(pipelineStateStream);
    }

    void Game::UploadResources(Scene::SceneObject* scene, SharedPtr<RenderContext> renderContext)
    {
        const auto& view = scene->GetRegistry().view<Scene::Components::MeshComponent>();

        auto stateTracker = MakeShared<ResourceStateTracker>(renderContext->GetGlobalResourceStateTracker());

        auto commandList = renderContext->CreateCopyCommandList();

        commandList->SetName(L"Uploading resources List");

        auto uploadBuffer = _mUploadBuffer[renderContext->GetCurrentBackBufferIndex()];
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

        std::vector<ID3D12CommandList*> commandLists;

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

    void Game::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh& mesh, const dx::XMMATRIX& world, Render::PassContext& passContext)
    {
        auto renderContext = passContext.renderContext;

        DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(world);
        auto d = DirectX::XMMatrixDeterminant(tWorld);
        DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
        tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);
        MeshUniform cb;
        DirectX::XMStoreFloat4x4(&cb.World, tWorld);
        DirectX::XMStoreFloat4x4(&cb.InverseTranspose, tWorldInverseTranspose);

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto dynamicDescriptorHeap = passContext.frameContext->dynamicDescriptorHeap;
        auto resourceStateTracker = passContext.resourceStateTracker;


        commandList->SetPipelineState(CreatePipelineState(mesh, renderContext).Get());

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

    void Game::Draw(Render::PassContext& passContext)
    {
        auto renderContext = passContext.renderContext;
        auto canvas = renderContext->GetSwapChain();

        auto commandList = passContext.commandList;
        commandList->SetName(L"Render scene List");

        renderContext->GetEventTracker().StartGPUEvent("Render scene list", commandList);

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto width = canvas->GetWidth();
        auto height = canvas->GetHeight();

        mDepthBufferDescriptor = renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->Allocate();
        CreateDepthBuffer(width, height, renderContext->Device());
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

        commandList->SetGraphicsRootSignature(mRootSignature->GetD3D12RootSignature().Get());

        passContext.frameContext->dynamicDescriptorHeap->ParseRootSignature(mRootSignature.get());

        float aspectRatio = canvas->GetWidth() / static_cast<float>(canvas->GetHeight());

        auto& registry = passContext.scene->GetRegistry();
        auto cameraEntity = registry.view<Scene::Components::CameraComponent>()[0];
        auto camera = registry.get<Scene::Components::CameraComponent>(cameraEntity).camera;
        auto cameraWT = registry.get<Scene::Components::WorldTransformComponent>(cameraEntity).transform;

        auto projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
        dx::XMVECTOR up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        dx::XMVECTOR forward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        dx::XMVECTOR sc;
        dx::XMVECTOR rt;
        dx::XMVECTOR tr;
        dx::XMMatrixDecompose(&sc, &rt, &tr, cameraWT);

        auto rotationMatrix = dx::XMMatrixRotationQuaternion(rt);
        auto lookAtDirection = dx::XMVector3Transform(
                forward,
                dx::XMMatrixRotationQuaternion(rt));

        using namespace DirectX;
        auto viewMatrix = dx::XMMatrixLookAtLH(tr, tr + lookAtDirection, up);

        DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
        mvpMatrix = DirectX::XMMatrixTranspose(mvpMatrix);

        FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, mvpMatrix);

        dx::XMStoreFloat3(&cb.EyePos, tr);

        const auto& lightsView = registry.view<Scene::Components::LightComponent, Scene::Components::WorldTransformComponent>();
        std::vector<LightUniform> lights;
        lights.reserve(lightsView.size());
        for (auto &&[entity, lightComponent, transformComponent] : lightsView.proxy())
        {
            LightUniform light = CommandListUtils::GetLightUniform(lightComponent.light, transformComponent.transform);

            lights.emplace_back(light);
        }

        cb.LightsCount = static_cast<uint32>(lights.size());

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(1, cbAllocation.GPU);

        auto lightsAllocation = passContext.frameContext->uploadBuffer->Allocate(lights.size() * sizeof(LightUniform), sizeof(LightUniform));

        lightsAllocation.CopyTo(lights);

        commandList->SetGraphicsRootShaderResourceView(3, lightsAllocation.GPU);

        dx::BoundingFrustum frustum(projectionMatrix);
        dx::XMStoreFloat4(&frustum.Orientation, rt);
        dx::XMStoreFloat3(&frustum.Origin, tr);

        const auto& meshsView = registry.view<Scene::Components::MeshComponent, Scene::Components::WorldTransformComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);
        for (auto &&[entity, meshComponent, transformComponent] : meshsView.proxy())
        {
            auto& aabb = registry.get<Scene::Components::AABBComponent>(entity);
            dx::BoundingBox b;
            aabb.boundingBox.Transform(b, transformComponent.transform);

            if (frustum.Intersects(b))
            {
                Draw(commandList, meshComponent.mesh, transformComponent.transform, passContext);
            }
        }

        renderContext->GetEventTracker().EndGPUEvent(commandList);
    }

    void Game::CreateDepthBuffer(int32 width, int32 height, ComPtr<ID3D12Device> device)
    {
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        ThrowIfFailed(device->CreateCommittedResource(
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

        device->CreateDepthStencilView(
            mDepthBuffer.Get(),
            &dsv,
            mDepthBufferDescriptor.GetDescriptor());

        mDepthBuffer->SetName(L"Depth buffer");
    }

} // namespace Engine