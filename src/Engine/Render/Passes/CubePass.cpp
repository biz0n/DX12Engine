#include "CubePass.h"

#include <ShaderTypes.h>

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

#include <Scene/Vertex.h>
#include <Scene/SceneObject.h>
#include <Scene/CubeMap.h>
#include <Scene/Camera.h>

#include <Memory/Texture.h>
#include <Memory/IndexBuffer.h>
#include <Memory/UploadBuffer.h>

#include <d3d12.h>

namespace Engine::Render::Passes
{
    CubePass::CubePass() : RenderPassBaseWithData<CubePassData>("Cube Pass")
    {

    }
    CubePass::~CubePass() = default;

    void CubePass::PrepareResources(Render::ResourcePlanner* planner)
    {
        float clear[4] = {0};
        Render::TextureCreationInfo rtTexture = {
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

    void CubePass::Render(Render::PassContext& passContext)
    {
        if (!PassData().hasCube)
        {
            return;
        }

        auto renderContext = passContext.renderContext;

        auto commandList = passContext.commandList;

        auto resourceStateTracker = passContext.resourceStateTracker;

        auto commandRecorder = passContext.commandRecorder;

        commandRecorder->SetViewPort();
        commandRecorder->SetRenderTargets({ResourceNames::ForwardOutput}, ResourceNames::ForwardDepth);

        commandRecorder->SetRootSignature(RootSignatureNames::Cube);
        commandRecorder->SetPipelineState(PSONames::Cube);

        auto& camera = PassData().camera;
        auto cb = CommandListUtils::GetFrameUniform(camera.viewProjection, camera.eyePosition, static_cast<uint32>(0));

        auto cbAllocation = passContext.frameContext->uploadBuffer->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto cubeMap = PassData().cube.cubeMap;

        auto cubeTexture = cubeMap.texture;

        auto cubeDescriptor = cubeTexture->GetCubeSRDescriptor().GetGPUDescriptor();

        commandList->SetGraphicsRootDescriptorTable(1, cubeDescriptor);

        commandList->IASetPrimitiveTopology(cubeMap.primitiveTopology);
        CommandListUtils::BindVertexBuffer(commandList, resourceStateTracker, *cubeMap.vertexBuffer);
        CommandListUtils::BindIndexBuffer(commandList, resourceStateTracker, *cubeMap.indexBuffer);

        commandList->DrawIndexedInstanced(static_cast<uint32>(cubeMap.indexBuffer->GetElementsCount()), 1, 0, 0, 0);
    }
} // namespace Engine::Render::Passes
