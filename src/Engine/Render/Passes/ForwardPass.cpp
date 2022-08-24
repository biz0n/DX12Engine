#include "ForwardPass.h"

#include <stdlib.h>
#include <Types.h>
#include <Render/ShaderTypes.h>

#include <Render/Passes/Names.h>

#include <HAL/SwapChain.h>

#include <Scene/SceneObject.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/Camera.h>
#include <Scene/PunctualLight.h>
#include <Scene/Vertex.h>
#include <Scene/SceneStorage.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Memory/TextureCreationInfo.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/PipelineStateStream.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/ShaderProvider.h>
#include <Render/PipelineStateProvider.h>
#include <Render/RootSignatureProvider.h>

#include <Render/RenderPassMediators/ResourcePlanner.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/Texture.h>
#include <Memory/UploadBuffer.h>
#include <Memory/MemoryForwards.h>
#include <Memory/IndexBuffer.h>

#include <DirectXTex.h>
#include <DirectXMath.h>
#include <d3d12.h>

namespace Engine::Render::Passes
{
    ForwardPass::ForwardPass() : Render::RenderPassBaseWithData<ForwardPassData>("Forward Pass", CommandQueueType::Graphics)
    {
    }

    ForwardPass::~ForwardPass() = default;

    void ForwardPass::CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider)
    {
        Render::RootSignatureBuilder builder = {};
        builder
            .AddConstantsParameter<int32>(0, 0)
            .AddCBVParameter(1, 0, D3D12_SHADER_VISIBILITY_ALL)
            .AddSRVParameter(0, 1, D3D12_SHADER_VISIBILITY_ALL)
            .AddSRVParameter(1, 1, D3D12_SHADER_VISIBILITY_PIXEL)
            .AddSRVParameter(2, 1, D3D12_SHADER_VISIBILITY_PIXEL);

        rootSignatureProvider->BuildRootSignature(RootSignatureNames::Forward, builder);
    }

    void ForwardPass::CreatePipelineStates(Render::PipelineStateProvider *pipelineStateProvider)
    {
        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;

        Render::PipelineStateProxy pipelineStateCullModeBack = {
            .rootSignatureName = RootSignatureNames::Forward,
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
        Memory::TextureCreationInfo dsTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(
                    DXGI_FORMAT_D32_FLOAT,
                    0,
                    0,
                    1,
                    1),
            .clearValue = D3D12_CLEAR_VALUE{.DepthStencil = {1.0, 0}}
        };
        planner->NewDepthStencil(ResourceNames::ForwardDepth, dsTexture);

        float clear[4] = {0, 0, 0, 0};
        Memory::TextureCreationInfo rtTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 0, 0, 1, 1),
            .clearValue = D3D12_CLEAR_VALUE{.Color = {0, 0, 0, 0}}
        };
        planner->NewRenderTarget(ResourceNames::ForwardOutput, rtTexture);

        planner->ReadRenderTarget(ResourceNames::ShadowDepth);
    }

    void ForwardPass::Draw(const MeshData& meshData, Render::PassRenderContext& passContext)
    {
        auto commandRecorder = passContext.commandRecorder;

        const auto& mesh = passContext.sceneStorage->GetMeshes()[meshData.meshIndex];
        const auto& meshUniform = passContext.sceneStorage->GetMeshUniforms()[meshData.meshIndex];

        const auto& material = passContext.sceneStorage->GetMaterials()[meshUniform.MaterialIndex];

        if (material.GetProperties().doubleSided)
        {
            commandRecorder->SetPipelineState(PSONames::ForwardCullNone);
        }
        else
        {
            commandRecorder->SetPipelineState(PSONames::ForwardCullBack);
        }

        commandRecorder->SetRoot32BitConstant(0, 0, meshData.meshIndex);

        commandRecorder->Draw(mesh.indicesCount, 0);
    }

    void ForwardPass::Render(Render::PassRenderContext &passContext)
    {
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();

        commandRecorder->SetRenderTargets({ResourceNames::ForwardOutput}, ResourceNames::ForwardDepth);

        commandRecorder->ClearRenderTargets({ResourceNames::ForwardOutput});
        commandRecorder->ClearDepthStencil(ResourceNames::ForwardDepth);

        commandRecorder->SetPipelineState(PSONames::ForwardCullBack);

        auto* depth = passContext.frameResourceProvider->GetTexture(ResourceNames::ShadowDepth);

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, static_cast<uint32>(passContext.sceneStorage->GetLightsCount()));
        cb.ShadowTransform = PassData().shadowTransform;
        cb.HasShadowTexture = passContext.sceneStorage->HasDirectionalLight();
        cb.ShadowIndex = depth->GetSRDescriptor().GetFullIndex();

        auto cbAllocation = passContext.uploadBuffer->Allocate(sizeof(Shader::FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(1, 0, cbAllocation.GPU);

        commandRecorder->SetRootShaderResourceView(0, 1, passContext.sceneStorage->GetMeshUniformsAllocation().GPU);

        commandRecorder->SetRootShaderResourceView(1, 1, passContext.sceneStorage->GetLightUniformsAllocation().GPU);

        commandRecorder->SetRootShaderResourceView(2, 1, passContext.sceneStorage->GetMaterialUniformsAllocation().GPU);

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker.get(), depth->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        auto& meshes = PassData().meshes;
        for (auto& mesh : meshes)
        {
            Draw(mesh, passContext);
        }
    }

} // namespace Engine