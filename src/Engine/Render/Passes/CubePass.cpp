#include "CubePass.h"

#include <Render/ShaderTypes.h>

#include <HAL/SwapChain.h>

#include <Render/Passes/Names.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/RootSignatureProvider.h>
#include <Render/PipelineStateStream.h>
#include <Render/PipelineStateProvider.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>

#include <Render/RenderPassMediators/PassCommandRecorder.h>
#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>
#include <Render/RenderRequest.h>

#include <Scene/SceneObject.h>

#include <Memory/TextureCreationInfo.h>
#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>
#include <Memory/DescriptorAllocatorPool.h>

#include <d3d12.h>

namespace Engine::Render::Passes
{
    CubePass::CubePass() : RenderPassBase("Cube Pass", CommandQueueType::Graphics)
    {

    }
    CubePass::~CubePass() = default;

    void CubePass::PrepareResources(Render::ResourcePlanner* planner)
    {
        float clear[4] = {0};
        Memory::TextureCreationInfo rtTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 0, 0, 1, 1),
            .clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, clear)
        };
        planner->WriteRenderTarget(ResourceNames::CubeOutput, ResourceNames::ForwardOutput);

        planner->ReadDeptStencil(ResourceNames::ForwardDepth);
    }

    void CubePass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        builder.AddConstantsParameter<uint32>(0, 0);
        builder.AddCBVParameter(1, 0);
        rootSignatureProvider->BuildRootSignature(RootSignatureNames::Cube, builder);
    }

    void CubePass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        CD3DX12_DEPTH_STENCIL_DESC dsDesc {D3D12_DEFAULT};
        dsDesc.DepthEnable = true;
        dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_FRONT;

        PipelineStateProxy pipelineState = {
            .rootSignatureName = RootSignatureNames::Cube,
            .vertexShaderName = Shaders::CubeVS,
            .pixelShaderName = Shaders::CubePS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R16G16B16A16_FLOAT
            },
            .rasterizer = rasterizer,
            .depthStencil = dsDesc
        };
        pipelineStateProvider->CreatePipelineState(PSONames::Cube, pipelineState);
    }

    void CubePass::Render(const RenderRequest& renderRequest, Render::PassRenderContext& passContext, const Timer& timer)
    {
        auto skyBoxTextureIndex = renderRequest.GetSceneStorage()->GetSceneData().skyBoxTextureIndex;
        
        if (!renderRequest.GetSceneStorage()->HasTexture(skyBoxTextureIndex))
        {
            return;
        }

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();
        commandRecorder->SetRenderTargets({ResourceNames::ForwardOutput}, ResourceNames::ForwardDepth);

        commandRecorder->SetPipelineState(PSONames::Cube);

        auto cubeDescriptorIndex = renderRequest.GetSceneStorage()->GetTexture(skyBoxTextureIndex)->GetCubeSRDescriptor().GetFullIndex();

        commandRecorder->SetRoot32BitConstant(0, 0, cubeDescriptorIndex);

        const auto& camera =renderRequest.GetCamera();
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, static_cast<uint32>(0));

        auto cbAllocation = passContext.uploadBuffer->Allocate(sizeof(Shader::FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(1, 0, cbAllocation.GPU);

        commandRecorder->Draw(36, 0);
    }
} // namespace Engine::Render::Passes
