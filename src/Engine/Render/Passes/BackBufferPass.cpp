#include "BackBufferPass.h"

#include <Render/Passes/Names.h>

#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/RootSignatureProvider.h>
#include <Render/PipelineStateStream.h>
#include <Render/PipelineStateProvider.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/FrameTransientContext.h>

#include <Memory/TextureCreationInfo.h>
#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

#include <imgui/imgui.h>
#include <Render/UIRenderContext.h>

namespace Engine::Render::Passes
{
    BackBufferPass::BackBufferPass() : RenderPassBase("Back Buffer Pass", CommandQueueType::Graphics)
    {

    }
    BackBufferPass::~BackBufferPass() = default;

    void BackBufferPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        planner->ReadRenderTarget(ResourceNames::ForwardOutput);
    }

    void BackBufferPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        builder.AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        rootSignatureProvider->BuildRootSignature(RootSignatureNames::BackBuffer, builder);
    }

    void BackBufferPass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        CD3DX12_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = false;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_FRONT;

        PipelineStateProxy pipelineState = {
            .rootSignatureName = RootSignatureNames::BackBuffer,
            .inputLayout = {},
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = Shaders::BackBufferVS,
            .pixelShaderName = Shaders::BackBufferPS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM
            },
            .rasterizer = rasterizer,
            .depthStencil = dsDesc
        };
        pipelineStateProvider->CreatePipelineState(PSONames::BackBuffer, pipelineState);
    }

    void BackBufferPass::Render(Render::PassRenderContext& passContext)
    {
        auto resourceStateTracker = passContext.resourceStateTracker;
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();

        commandRecorder->SetBackBufferAsRenderTarget();

        commandRecorder->SetPipelineState(PSONames::BackBuffer);
        
        auto* color = passContext.frameResourceProvider->GetTexture(ResourceNames::TonemappingOutput);
        auto colorSRV = color->GetSRDescriptor().GetGPUDescriptor();

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker.get(), color->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        commandRecorder->SetRootDescriptorTable(0, 0, colorSRV);

        commandRecorder->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        commandRecorder->Draw(3, 0);
    }
} // namespace Engine::Render::Passes
