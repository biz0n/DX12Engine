#include "ToneMappingPass.h"

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
    ToneMappingPass::ToneMappingPass() : RenderPassBase("Tone Mapping Pass", CommandQueueType::Compute)
    {

    }
    ToneMappingPass::~ToneMappingPass() = default;

    void ToneMappingPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        planner->ReadRenderTarget(ResourceNames::ForwardOutput);

        Memory::TextureCreationInfo texture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(
                    DXGI_FORMAT_R16G16B16A16_FLOAT,
                    0,
                    0,
                    1,
                    1,
                    1,
                    0,
                    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        };
        planner->NewTexture(ResourceNames::TonemappingOutput, texture);
    }

    void ToneMappingPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        rootSignatureProvider->BuildRootSignature(RootSignatureNames::ToneMapping, builder);
    }

    void ToneMappingPass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        ComputePipelineStateProxy pipelineState = {
            .rootSignatureName = RootSignatureNames::ToneMapping,
            .computeShaderName = Shaders::ToneMapCS
        };
        pipelineStateProvider->CreatePipelineState(PSONames::ToneMapping, pipelineState);
    }

    void ToneMappingPass::Render(Render::PassRenderContext& passContext)
    {
        auto resourceStateTracker = passContext.resourceStateTracker;
        auto commandRecorder = passContext.commandRecorder;

       // commandRecorder->SetViewPort();

        //commandRecorder->SetBackBufferAsRenderTarget();

        commandRecorder->SetPipelineState(PSONames::ToneMapping);
        
        auto* inputTexture = passContext.frameResourceProvider->GetTexture(ResourceNames::ForwardOutput);
        auto* outputTexture = passContext.frameResourceProvider->GetTexture(ResourceNames::TonemappingOutput);
       // auto colorSRV = color->GetSRDescriptor().GetGPUDescriptor();

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker.get(), inputTexture->D3DResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker.get(), outputTexture->D3DResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        struct
        {
            uint32 InputTexIndex;
            uint32 OutputTexIndex;
        } cbData;

        cbData.InputTexIndex = inputTexture->GetSRDescriptor().GetIndex();
        cbData.OutputTexIndex = outputTexture->GetUADescriptor().GetIndex();
        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(cbData));
        cbAllocation.CopyTo(&cbData, sizeof(cbData));
        commandRecorder->SetRootConstantBufferView(0, 10, cbAllocation.GPU);

        uint32 x = std::ceilf(outputTexture->GetDescription().Width / 16.0f);
        uint32 y = std::ceilf(outputTexture->GetDescription().Height / 16.0f);

        commandRecorder->Dispatch(x, y);

        commandRecorder->UAVBarrier(outputTexture->D3DResource());

       // commandRecorder->SetRootDescriptorTable(0, 0, colorSRV);

       // commandRecorder->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

       // commandRecorder->Draw(3, 0);

    //    auto* depth = passContext.frameResourceProvider->GetTexture(ResourceNames::ShadowDepth);

     //   CommandListUtils::TransitionBarrier(passContext.resourceStateTracker.get(), depth->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

     //   auto depsDescriptor = depth->GetSRDescriptor().GetGPUDescriptor();

     //   ImGui::Begin("ShadowMap");
    //    ImGui::Image(IMGUI_TEXTURE_ID(depsDescriptor), {512, 512});
    //    ImGui::End();

    }
} // namespace Engine::Render::Passes
