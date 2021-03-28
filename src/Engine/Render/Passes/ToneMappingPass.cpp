#include "ToneMappingPass.h"

#include <Render/Passes/Names.h>

#include <Render/RootSignatureBuilder.h>
#include <Render/CommandListUtils.h>
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

#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

#include <imgui/imgui.h>
#include <Render/UIRenderContext.h>

namespace Engine::Render::Passes
{
    ToneMappingPass::ToneMappingPass() : RenderPassBase("Tone Mapping Pass")
    {

    }
    ToneMappingPass::~ToneMappingPass() = default;

    void ToneMappingPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        planner->ReadRenderTarget(ResourceNames::ForwardOutput);
    }

    void ToneMappingPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        builder.AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        rootSignatureProvider->BuildRootSignature(RootSignatureNames::ToneMapping, builder);
    }

    void ToneMappingPass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        CD3DX12_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = false;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_FRONT;

        PipelineStateProxy pipelineState = {
            .rootSignatureName = RootSignatureNames::ToneMapping,
            .inputLayout = {},
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = Shaders::TonemapVS,
            .pixelShaderName = Shaders::ToneMapPS,
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM
            },
            .rasterizer = rasterizer,
            .depthStencil = dsDesc
        };
        pipelineStateProvider->CreatePipelineState(PSONames::ToneMapping, pipelineState);
    }

    void ToneMappingPass::Render(Render::PassContext& passContext)
    {
        auto renderContext = passContext.renderContext;
        auto canvas = renderContext->GetSwapChain();

        auto commandList = passContext.commandList;

        auto resourceStateTracker = passContext.resourceStateTracker;
        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();

        commandRecorder->SetBackBufferAsRenderTarget();

        commandRecorder->SetRootSignature(RootSignatureNames::ToneMapping);
        commandRecorder->SetPipelineState(PSONames::ToneMapping);
        
        auto* color = passContext.frameResourceProvider->GetTexture(ResourceNames::ForwardOutput);
        auto colorSRV = color->GetSRDescriptor().GetGPUDescriptor();

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker, color->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        commandList->SetGraphicsRootDescriptorTable(0, colorSRV);

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        auto allocation = passContext.frameContext->uploadBuffer->Allocate(3 * sizeof(uint16), 1);
        uint16 indices[3] = {0, 1, 2};
        allocation.CopyTo(&indices);

        D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        indexBufferView.SizeInBytes = 3 * sizeof(uint16);
        indexBufferView.BufferLocation = allocation.GPU;

        commandList->IASetIndexBuffer(&indexBufferView);


        commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

        auto* depth = passContext.frameResourceProvider->GetTexture(ResourceNames::ShadowDepth);

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker, depth->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        auto depsDescriptor = depth->GetSRDescriptor().GetGPUDescriptor();

        ImGui::Begin("ShadowMap");
        ImGui::Image(IMGUI_TEXTURE_ID(depsDescriptor), {512, 512});
        ImGui::End();

    }
} // namespace Engine::Render::Passes
