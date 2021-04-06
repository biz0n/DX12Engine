#include "DepthPass.h"

#include <Render/Passes/Names.h>

#include <EngineConfig.h>

#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/Vertex.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Memory/TextureCreationInfo.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>
#include <Render/RootSignatureProvider.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/PipelineStateStream.h>
#include <Render/PipelineStateProvider.h>
#include <HAL/SwapChain.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/FrameTransientContext.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>

#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

namespace Engine::Render::Passes
{
    DepthPass::DepthPass() : Render::RenderPassBaseWithData<DepthPassData>("Depth Pass", CommandQueueType::Graphics)
    {

    }
    DepthPass::~DepthPass() = default;

    void DepthPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        Memory::TextureCreationInfo dsTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, EngineConfig::ShadowWidth, EngineConfig::ShadowHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
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
            .AddCBVParameter(2, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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

    void DepthPass::Render(Render::PassRenderContext& passContext)
    {
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetPipelineState(PSONames::Depth);

        commandRecorder->SetViewPort(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);

        commandRecorder->SetRenderTargets({}, ResourceNames::ShadowDepth);

        commandRecorder->ClearDepthStencil(ResourceNames::ShadowDepth);

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, 0);

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(Shader::FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(1, 0, cbAllocation.GPU);

        auto& meshes = PassData().meshes;
        for (auto &mesh : meshes)
        {
            Draw(mesh.mesh, mesh.worldTransform, passContext);
        }
    }

    void DepthPass::Draw(const Scene::Mesh &mesh, const dx::XMMATRIX &world, Render::PassRenderContext &passContext)
    {
        auto commandRecorder = passContext.commandRecorder;

        auto cb = CommandListUtils::GetMeshUniform(world);
        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(Shader::MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(0, 0, cbAllocation.GPU);

        auto resourceStateTracker = passContext.resourceStateTracker;

        commandRecorder->IASetPrimitiveTopology(mesh.primitiveTopology);

        Shader::MaterialUniform uniform = CommandListUtils::GetMaterialUniform(*mesh.material.get());

        auto matAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(Shader::MaterialUniform));
        matAllocation.CopyTo(&uniform);
        commandRecorder->SetRootConstantBufferView(2, 0, matAllocation.GPU);

        if (mesh.material->HasBaseColorTexture())
        {
            CommandListUtils::TransitionBarrier(resourceStateTracker.get(), mesh.material->GetBaseColorTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }

        CommandListUtils::BindVertexBuffer(commandRecorder.get(), resourceStateTracker, mesh.vertexBuffer.get());
        CommandListUtils::BindIndexBuffer(commandRecorder.get(), resourceStateTracker, mesh.indexBuffer.get());

        commandRecorder->DrawIndexed(0, static_cast<uint32>(mesh.indexBuffer->GetElementsCount()), 0);
    }
} // namespace Engine::Render::Passes
