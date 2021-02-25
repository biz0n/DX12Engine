#include "DepthPass.h"

#include <Render/Passes/Names.h>

#include <EngineConfig.h>

#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/Texture.h>
#include <Scene/Vertex.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/CommandListUtils.h>
#include <Render/Texture.h>
#include <Render/TextureCreationInfo.h>
#include <Render/ResourcePlanner.h>
#include <Render/RootSignatureProvider.h>
#include <Render/PassContext.h>
#include <Render/PipelineStateStream.h>
#include <Render/PipelineStateProvider.h>
#include <Render/SwapChain.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/FrameTransientContext.h>
#include <Render/PassCommandRecorder.h>

#include <Memory/IndexBuffer.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/UploadBuffer.h>

namespace Engine::Render::Passes
{
    DepthPass::DepthPass() : Render::RenderPassBaseWithData<DepthPassData>("Depth Pass")
    {

    }
    DepthPass::~DepthPass() = default;

    void DepthPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        Render::TextureCreationInfo dsTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, EngineConfig::ShadowWidth, EngineConfig::ShadowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            .clearValue = optimizedClearValue
        };
        planner->NewDepthStencil(ResourceNames::ShadowDepth, dsTexture);
    }

    void DepthPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        Render::RootSignatureBuilder builder = {};
        builder
            .AddCBVParameter(0, 0, D3D12_SHADER_VISIBILITY_VERTEX)
            .AddCBVParameter(1, 0, D3D12_SHADER_VISIBILITY_ALL)
            .AddCBVParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        rootSignatureProvider->BuildRootSignature(RootSignatureNames::Depth, builder);
    }

    void DepthPass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;
        rasterizer.DepthBias = 50000;
        rasterizer.DepthBiasClamp = 0.0f;
        rasterizer.SlopeScaledDepthBias = 1.0f;

        PipelineStateProxy pipelineState = {
            .rootSignatureName = RootSignatureNames::Depth,
            .inputLayout = Scene::Vertex::GetInputLayout(),
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = Shaders::DepthVS,
            .pixelShaderName = Shaders::DepthPS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {},
            .rasterizer = rasterizer,
            .depthStencil = CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}
        };
        pipelineStateProvider->CreatePipelineState(PSONames::Depth, pipelineState);
    }

    void DepthPass::Render(Render::PassContext& passContext)
    {
        auto renderContext = passContext.renderContext;

        auto commandList = passContext.commandList;

        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);

        commandRecorder->SetRenderTargets({}, ResourceNames::ShadowDepth);

        commandRecorder->ClearDepthStencil(ResourceNames::ShadowDepth);

        commandRecorder->SetRootSignature(RootSignatureNames::Depth);

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, 0);

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(1, cbAllocation.GPU);

        auto& meshes = PassData().meshes;
        for (auto &mesh : meshes)
        {
            Draw(commandList, mesh.mesh, mesh.worldTransform, passContext);
        }
    }

    void DepthPass::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const Scene::Mesh &mesh, const dx::XMMATRIX &world, Render::PassContext &passContext)
    {
        auto renderContext = passContext.renderContext;
        auto commandRecorder = passContext.commandRecorder;

        auto cb = CommandListUtils::GetMeshUniform(world);
        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto dynamicDescriptorHeap = passContext.frameContext->dynamicDescriptorHeap;
        auto resourceStateTracker = passContext.resourceStateTracker;

        commandRecorder->SetPipelineState(PSONames::Depth);

        commandList->IASetPrimitiveTopology(mesh.primitiveTopology);

        

        auto device = renderContext->Device();
        auto descriptorAllocator = renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        MaterialUniform uniform = CommandListUtils::GetMaterialUniform(*mesh.material.get());

        auto matAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(MaterialUniform));
        matAllocation.CopyTo(&uniform);
        commandList->SetGraphicsRootConstantBufferView(2, matAllocation.GPU);

        if (mesh.material->HasBaseColorTexture())
        {
            CommandListUtils::TransitionBarrier(resourceStateTracker, mesh.material->GetBaseColorTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(3, 0, 1, mesh.material->GetBaseColorTexture()->GetShaderResourceView(device, descriptorAllocator));
        }

        CommandListUtils::BindVertexBuffer(commandList, resourceStateTracker, *mesh.vertexBuffer);
        CommandListUtils::BindIndexBuffer(commandList, resourceStateTracker, *mesh.indexBuffer);

        dynamicDescriptorHeap->CommitStagedDescriptors(renderContext->Device(), commandList);

        commandList->DrawIndexedInstanced(static_cast<uint32>(mesh.indexBuffer->GetElementsCount()), 1, 0, 0, 0);
    }
} // namespace Engine::Render::Passes
