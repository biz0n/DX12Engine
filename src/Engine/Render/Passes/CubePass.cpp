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
#include <Render/FrameTransientContext.h>

#include <Render/RenderPassMediators/PassCommandRecorder.h>
#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Render/RenderPassMediators/PassRenderContext.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>

#include <Scene/Vertex.h>
#include <Scene/SceneObject.h>
#include <Scene/CubeMap.h>
#include <Scene/Camera.h>

#include <Memory/TextureCreationInfo.h>
#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

#include <d3d12.h>

namespace Engine::Render::Passes
{
    CubePass::CubePass() : RenderPassBaseWithData<CubePassData>("Cube Pass", CommandQueueType::Graphics)
    {

    }
    CubePass::~CubePass() = default;

    void CubePass::PrepareResources(Render::ResourcePlanner* planner)
    {
        float clear[4] = {0};
        Memory::TextureCreationInfo rtTexture = {
            .description = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 0, 0, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
            .clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, clear)
        };
        planner->NewRenderTarget(ResourceNames::CubeOutput, rtTexture);

        planner->ReadDeptStencil(ResourceNames::ForwardDepth);
    }

    void CubePass::CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider)
    {
        RootSignatureBuilder builder = {};
        builder.AddCBVParameter(0, 0);
        builder.AddSRVDescriptorTableParameter(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
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
            .inputLayout = Scene::Vertex::GetInputLayout(),
            .primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
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

    void CubePass::Render(Render::PassRenderContext& passContext)
    {
        if (!PassData().hasCube)
        {
            return;
        }

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();
        commandRecorder->SetRenderTargets({ResourceNames::ForwardOutput}, ResourceNames::ForwardDepth);

        commandRecorder->SetPipelineState(PSONames::Cube);

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, static_cast<uint32>(0));

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(Shader::FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandRecorder->SetRootConstantBufferView(0, 0, cbAllocation.GPU);

        auto cubeMap = PassData().cube.cubeMap;

        auto cubeTexture = cubeMap.texture;

        auto cubeDescriptor = cubeTexture->GetCubeSRDescriptor().GetGPUDescriptor();

        commandRecorder->SetRootDescriptorTable(0, 0, cubeDescriptor);

        commandRecorder->IASetPrimitiveTopology(cubeMap.primitiveTopology);
        CommandListUtils::BindVertexBuffer(commandRecorder.get(), resourceStateTracker, cubeMap.vertexBuffer.get());
        CommandListUtils::BindIndexBuffer(commandRecorder.get(), resourceStateTracker, cubeMap.indexBuffer.get());

        commandRecorder->DrawIndexed(0, static_cast<uint32>(cubeMap.indexBuffer->GetElementsCount()), 0);
    }
} // namespace Engine::Render::Passes
