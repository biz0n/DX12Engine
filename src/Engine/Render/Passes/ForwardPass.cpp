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
#include <Render/PassCommandRecorder.h>

#include <Memory/UploadBuffer.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/IndexBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>

#include <DirectXTex.h>
#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine::Render::Passes
{
    ForwardPass::ForwardPass() : Render::RenderPassBaseWithData<ForwardPassData>("Forward Pass")
    {
    }

    ForwardPass::~ForwardPass() = default;

    void ForwardPass::CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider)
    {
        Render::RootSignatureBuilder builder = {};
        builder
            .AddCBVParameter(0, 0, D3D12_SHADER_VISIBILITY_VERTEX)
            .AddCBVParameter(1, 0, D3D12_SHADER_VISIBILITY_ALL)
            .AddCBVParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVParameter(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(1, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(3, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(4, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(5, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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
        planner->NewRenderTarget(ResourceNames::ForwardOutput, rtTexture);
    }

    void ForwardPass::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh &mesh, const dx::XMMATRIX &world, Render::PassContext &passContext)
    {
        auto renderContext = passContext.renderContext;
        auto commandRecorder = passContext.commandRecorder;

        auto cb = CommandListUtils::GetMeshUniform(world);
        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto dynamicDescriptorHeap = passContext.frameContext->dynamicDescriptorHeap;
        auto resourceStateTracker = passContext.resourceStateTracker;

        if (mesh.material->GetProperties().doubleSided)
        {
            commandRecorder->SetPipelineState(PSONames::ForwardCullNone);
        }
        else
        {
            commandRecorder->SetPipelineState(PSONames::ForwardCullBack);
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

        auto commandList = passContext.commandList;

        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();

        commandRecorder->SetRenderTargets({ResourceNames::ForwardOutput}, ResourceNames::ForwardDepth);

        commandRecorder->ClearRenderTargets({ResourceNames::ForwardOutput});
        commandRecorder->ClearDepthStencil(ResourceNames::ForwardDepth);

        commandRecorder->SetRootSignature(RootSignatureNames::Forward);

        auto& lightsData = PassData().lights;
        std::vector<LightUniform> lights;
        lights.reserve(lightsData.size());

        for (auto& lightData : lightsData)
        {
            LightUniform light = CommandListUtils::GetLightUniform(lightData.light, lightData.worldTransform);
            lights.emplace_back(light);
        }

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, static_cast<uint32>(lights.size()));
        cb.ShadowTransform = PassData().shadowTransform;

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(1, cbAllocation.GPU);

        auto lightsAllocation = passContext.frameContext->uploadBuffer->Allocate(lights.size() * sizeof(LightUniform), sizeof(LightUniform));

        lightsAllocation.CopyTo(lights);

        commandList->SetGraphicsRootShaderResourceView(3, lightsAllocation.GPU);

        auto* depth = passContext.frameResourceProvider->GetTexture(ResourceNames::ShadowDepth);

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker, depth->D3D12ResourceCom(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.TextureCube.MostDetailedMip = 0;
        desc.TextureCube.MipLevels = depth->D3D12Resource()->GetDesc().MipLevels;
        desc.TextureCube.ResourceMinLODClamp = 0.0f;
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        auto srv = depth->GetSRDescriptor(renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).get(), &desc);

        passContext.frameContext->dynamicDescriptorHeap->StageDescriptor(9, 0, 1, srv);

        auto& meshes = PassData().meshes;
        for (auto &mesh : meshes)
        {
            Draw(commandList, mesh.mesh, mesh.worldTransform, passContext);
        }
    }

} // namespace Engine