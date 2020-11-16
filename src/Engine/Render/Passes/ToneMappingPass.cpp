#include "ToneMappingPass.h"

#include <Render/RootSignatureBuilder.h>
#include <Render/PipelineStateStream.h>

#include <Render/CommandListUtils.h>
#include <Memory/IndexBuffer.h>

#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/UploadBuffer.h>

namespace Engine::Render::Passes
{
    ToneMappingPass::ToneMappingPass() : RenderPassBase("Tone Mapping Pass")
    {

    }
    ToneMappingPass::~ToneMappingPass() = default;

    void ToneMappingPass::PrepareResources(Render::ResourcePlanner* planner)
    {
        planner->ReadRenderTarget("ForwardRT");
    }

    void ToneMappingPass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        builder.AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        rootSignatureProvider->BuildRootSignature("ToneMappingRS", builder);
    }

    void ToneMappingPass::CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider)
    {
        CD3DX12_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = false;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_FRONT;

        PipelineStateProxy pipelineState = {
            .rootSignatureName = "ToneMappingRS",
            .inputLayout = {},
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .vertexShaderName = "Resources\\Shaders\\ScreenVS.hlsl",
            .pixelShaderName = "Resources\\Shaders\\TonemappingPS.hlsl",
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .rtvFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM
            },
            .rasterizer = rasterizer,
            .depthStencil = dsDesc
        };
        pipelineStateProvider->CreatePipelineState("ToneMappingPipelineState", pipelineState);
    }

    void ToneMappingPass::Render(Render::PassContext& passContext)
    {
        auto renderContext = passContext.renderContext;
        auto canvas = renderContext->GetSwapChain();

        auto commandList = passContext.commandList;

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto width = canvas->GetWidth();
        auto height = canvas->GetHeight();

        auto backBuffer = canvas->GetCurrentBackBuffer();
        CommandListUtils::TransitionBarrier(resourceStateTracker, backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);


        auto rtv = canvas->GetCurrentRenderTargetView();

        auto screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        auto scissorRect = CD3DX12_RECT(0, 0, width, height);

        commandList->RSSetViewports(1, &screenViewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        auto rootSignature = renderContext->GetRootSignatureProvider()->GetRootSignature("ToneMappingRS");
        commandList->SetGraphicsRootSignature(rootSignature->GetD3D12RootSignature().Get());

        commandList->SetPipelineState(renderContext->GetPipelineStateProvider()->GetPipelineState("ToneMappingPipelineState").Get());

        passContext.frameContext->dynamicDescriptorHeap->ParseRootSignature(rootSignature);

        auto* color = passContext.frameResourceProvider->GetTexture("CubeRT");
        auto colorSRV = color->GetSRDescriptor(renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).get());

        CommandListUtils::TransitionBarrier(passContext.resourceStateTracker, color->D3D12ResourceCom(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        passContext.frameContext->dynamicDescriptorHeap->StageDescriptor(0, 0, 1, colorSRV);

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        auto allocation = passContext.frameContext->uploadBuffer->Allocate(3 * sizeof(uint16), 1);
        uint16 indices[3] = {0, 1, 2};
        allocation.CopyTo(&indices);

        D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        indexBufferView.SizeInBytes = 3 * sizeof(uint16);
        indexBufferView.BufferLocation = allocation.GPU;

        passContext.frameContext->dynamicDescriptorHeap->CommitStagedDescriptors(renderContext->Device(), commandList);

        commandList->IASetIndexBuffer(&indexBufferView);


        commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
    }
} // namespace Engine::Render::Passes
