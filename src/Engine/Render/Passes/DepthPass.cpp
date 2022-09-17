#include "DepthPass.h"

#include <Render/Passes/Names.h>

#include <EngineConfig.h>

#include <Scene/MeshResources.h>
#include <Scene/SceneStorage.h>

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
#include <Render/RenderPassMediators/PassCommandRecorder.h>
#include <Render/RenderRequest.h>

#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

namespace Engine::Render::Passes
{
    DepthPass::DepthPass() : Render::RenderPassBase("Depth Pass", CommandQueueType::Graphics)
    {

    }
    DepthPass::~DepthPass() = default;

    void DepthPass::PrepareResources(Render::ResourcePlanner* planner)
    {

        Memory::TextureCreationInfo dsTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, EngineConfig::ShadowWidth, EngineConfig::ShadowHeight, 1, 1),
            .clearValue = D3D12_CLEAR_VALUE{.DepthStencil = {1.0, 0}}
        };
        planner->NewDepthStencil(ResourceNames::ShadowDepth, dsTexture);
    }

    void DepthPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        Render::RootSignatureBuilder builder = {};
        builder
            .AddConstantsParameter<int32>(0, 0)
            .AddCBVParameter(1, 0, D3D12_SHADER_VISIBILITY_ALL)
            .AddSRVParameter(0, 1, D3D12_SHADER_VISIBILITY_ALL)
            .AddSRVParameter(1, 1, D3D12_SHADER_VISIBILITY_PIXEL);

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
            .vertexShaderName = Shaders::DepthVS,
            .pixelShaderName = Shaders::DepthPS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {},
            .rasterizer = rasterizer,
            .depthStencil = CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}
        };
        pipelineStateProvider->CreatePipelineState(PSONames::Depth, pipelineState);
    }

    void DepthPass::Render(const RenderRequest& renderRequest, Render::PassRenderContext& passContext, const Timer& timer)
    {
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetPipelineState(PSONames::Depth);

        commandRecorder->SetViewPort(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);

        commandRecorder->SetRenderTargets({}, ResourceNames::ShadowDepth);

        commandRecorder->ClearDepthStencil(ResourceNames::ShadowDepth);

        if (renderRequest.GetShadowCameras().empty())
        {
            return;
        }

        const auto& camera = renderRequest.GetShadowCameras()[0];
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, 0);

        auto cbAllocation = passContext.uploadBuffer->Allocate(sizeof(Shader::FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(1, 0, cbAllocation.GPU);

        commandRecorder->SetRootShaderResourceView(0, 1, renderRequest.GetMeshAllocation().GPU);
        commandRecorder->SetRootShaderResourceView(1, 1, renderRequest.GetMaterialAllocation().GPU);

        for (auto meshIndex : renderRequest.GetMeshes().opaque)
        {
            Draw(renderRequest, meshIndex, passContext);
        }
    }

    void DepthPass::Draw(const RenderRequest& renderRequest, Index meshIndex, Render::PassRenderContext &passContext)
    {
        const auto& meshUniform = renderRequest.GetMeshes().meshes[meshIndex];
        const auto& mesh = renderRequest.GetSceneStorage()->GetMeshes()[meshUniform.Id];
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetRoot32BitConstant(0, 0, meshIndex);

        commandRecorder->Draw(mesh.GetIndicesCount(), 0);
    }
} // namespace Engine::Render::Passes
