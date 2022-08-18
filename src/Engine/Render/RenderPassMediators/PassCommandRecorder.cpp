#include "PassCommandRecorder.h"

#include <HAL/SwapChain.h>
#include <HAL/RootSignature.h>

#include <Render/RenderPassMediators/CommandListUtils.h>
#include <Render/RenderPassMediators/PassContext.h>
#include <Render/RenderPassMediators/RenderPassBase.h>

#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/RootSignatureProvider.h>
#include <Render/FrameTransientContext.h>
#include <Render/PipelineStateProvider.h>

#include <Memory/Texture.h>
#include <Memory/ResourceStateTracker.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>

#include <d3dx12.h>

namespace Engine::Render
{
    PassCommandRecorder::PassCommandRecorder(
        PassContext* passContext,
        ComPtr<ID3D12GraphicsCommandList> commandList,
        Memory::ResourceStateTracker *resourceStateTracker,
        RenderContext *renderContext,
        const FrameResourceProvider *frameResourceProvider,
        FrameTransientContext* frameTransientContext) : mPassContext{passContext}, 
                                                        mCommandList(commandList),
                                                        mResourceStateTracker(resourceStateTracker),
                                                        mRenderContext(renderContext),
                                                        mFrameResourceProvider(frameResourceProvider),
                                                        mFrameTransientContext(frameTransientContext)
    {
    }

    PassCommandRecorder::~PassCommandRecorder() = default;

    void PassCommandRecorder::SetViewPort(uint32 width, uint32 height)
    {
        if (width == 0 || height == 0)
        {
            width = mRenderContext->GetSwapChain()->GetWidth();
            height = mRenderContext->GetSwapChain()->GetHeight();
        }

        auto screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        auto scissorRect = CD3DX12_RECT(0, 0, width, height);

        mCommandList->RSSetViewports(1, &screenViewport);
        mCommandList->RSSetScissorRects(1, &scissorRect);
    }

    void PassCommandRecorder::SetRenderTargets(const std::vector<Name>& renderTargets, const Name& depthStencil)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE dsDescriptor {0};

        if (depthStencil.isValid())
        {
            Memory::Texture* dsTexture = mFrameResourceProvider->GetTexture(depthStencil);
            
            CommandListUtils::TransitionBarrier(mResourceStateTracker, dsTexture->D3DResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

            dsDescriptor = dsTexture->GetDSDescriptor().GetCPUDescriptor();
        }

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
        renderTargetDescriptors.reserve(renderTargets.size());

        for (const auto& renderTargetName : renderTargets)
        {
            Memory::Texture* rtTexture = mFrameResourceProvider->GetTexture(renderTargetName);

            CommandListUtils::TransitionBarrier(mResourceStateTracker, rtTexture->D3DResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

            auto rtDescriptor = rtTexture->GetRTDescriptor().GetCPUDescriptor();

            renderTargetDescriptors.push_back(rtDescriptor);
        }

        mCommandList->OMSetRenderTargets(static_cast<uint32>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), false, ((dsDescriptor.ptr != 0) ? &dsDescriptor : nullptr));
    }

    void PassCommandRecorder::SetBackBufferAsRenderTarget()
    {
        auto backBufferTexture = mRenderContext->GetSwapChain()->GetCurrentBackBufferTexture();
        auto backBuffer = backBufferTexture->D3DResource();
        CommandListUtils::TransitionBarrier(mResourceStateTracker, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

        auto rtv = backBufferTexture->GetRTDescriptor().GetCPUDescriptor();

        mCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);
    }

    void PassCommandRecorder::ClearRenderTargets(const std::vector<Name>& renderTargets)
    {
        for (const auto& renderTargetName : renderTargets)
        {
            Memory::Texture* rtTexture = mFrameResourceProvider->GetTexture(renderTargetName);

            auto rtDescriptor = rtTexture->GetRTDescriptor().GetCPUDescriptor();

            mCommandList->ClearRenderTargetView(rtDescriptor, rtTexture->GetClearValue().Color, 0, nullptr);
        }
    }

    void PassCommandRecorder::ClearDepthStencil(const Name& depthStencil)
    {
        Memory::Texture* dsTexture = mFrameResourceProvider->GetTexture(depthStencil);
        auto dsDescriptor = dsTexture->GetDSDescriptor().GetCPUDescriptor();
        const auto& clearValue = dsTexture->GetClearValue();

        mCommandList->ClearDepthStencilView(dsDescriptor, D3D12_CLEAR_FLAG_DEPTH, clearValue.DepthStencil.Depth,  clearValue.DepthStencil.Stencil, 0, nullptr);
    }
    
    void PassCommandRecorder::SetRootSignature(const Name& rootSignature)
    {
        if (mLastRootSignatureName == rootSignature)
        {
            return;
        }

        ID3D12DescriptorHeap* heaps[2] = {
            mRenderContext->GetDescriptorAllocator()->GetCbvSrvUavDescriptorHeap(), 
            mRenderContext->GetDescriptorAllocator()->GetSamplerDescriptorHeap()
        };
        mCommandList->SetDescriptorHeaps(std::size(heaps), heaps);

        auto rs = mRenderContext->GetRootSignatureProvider()->GetRootSignature(rootSignature);

        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootSignature(rs->GetD3D12RootSignature().Get());
        }
        else
        {
            mCommandList->SetComputeRootSignature(rs->GetD3D12RootSignature().Get());
        }

        mLastRootSignatureName = rootSignature;
        mLastRootSignature = rs;

        auto srDescriptorHandle = mRenderContext->GetDescriptorAllocator()->GetSRDescriptorHandle();
        auto auDescriptorHandle = mRenderContext->GetDescriptorAllocator()->GetUADescriptorHandle();
        auto samplerDescriptorHandle = mRenderContext->GetDescriptorAllocator()->GetSamplerDescriptorHandle();

        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootDescriptorTable(rs->GetIndex(HAL::RootSignature::RegisterType::Sampler, 0, 10), samplerDescriptorHandle);
            mCommandList->SetGraphicsRootDescriptorTable(rs->GetIndex(HAL::RootSignature::RegisterType::Sampler, 0, 11), samplerDescriptorHandle);
        }
        else 
        {
            //mCommandList->SetComputeRootDescriptorTable(rs->GetIndex(HAL::RootSignature::RegisterType::Sampler, 0, 10), samplerDescriptorHandle);
            //mCommandList->SetComputeRootDescriptorTable(rs->GetIndex(HAL::RootSignature::RegisterType::Sampler, 0, 11), samplerDescriptorHandle);
        }
    }

    void PassCommandRecorder::SetPipelineState(const Name& pso)
    {
        if (mLastPSOName == pso)
        {
            return;
        }

        mCommandList->SetPipelineState(mRenderContext->GetPipelineStateProvider()->GetPipelineState(pso).Get());
        auto rootSignature = mRenderContext->GetPipelineStateProvider()->GetAssociatedRootSignatureName(pso);
        
        mLastPSOName = pso;

        SetRootSignature(rootSignature);
    }

    void PassCommandRecorder::SetRoot32BitConstant(uint32 registerIndex, uint32 registerSpace, int32 value, uint32 destOffsetIn32BitValue)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::ConstantBuffer, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRoot32BitConstant(parameterIndex, value, destOffsetIn32BitValue);
        }
        else
        {
            mCommandList->SetComputeRoot32BitConstant(parameterIndex, value, destOffsetIn32BitValue);
        }
    }

    void PassCommandRecorder::SetRoot32BitConstants(uint32 registerIndex, uint32 registerSpace, uint32 num32BitValuesToSet, const void *srcData, uint32 destOffsetIn32BitValues)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::ConstantBuffer, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRoot32BitConstants(parameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
        }
        else
        {
            mCommandList->SetComputeRoot32BitConstants(parameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
        }
    }

    void PassCommandRecorder::SetRootConstantBufferView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::ConstantBuffer, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootConstantBufferView(parameterIndex, bufferLocation);
        }
        else
        {
            mCommandList->SetComputeRootConstantBufferView(parameterIndex, bufferLocation);
        }
    }

    void PassCommandRecorder::SetRootShaderResourceView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::ShaderResource, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootShaderResourceView(parameterIndex, bufferLocation);
        }
        else
        {
            mCommandList->SetComputeRootShaderResourceView(parameterIndex, bufferLocation);
        }
    }

    void PassCommandRecorder::SetRootUnorderedAccessView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::UnorderedAccess, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootUnorderedAccessView(parameterIndex, bufferLocation);
        }
        else
        {
            mCommandList->SetComputeRootUnorderedAccessView(parameterIndex, bufferLocation);
        }
    }

    void PassCommandRecorder::SetRootDescriptorTable(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle)
    {
        assert(mLastRootSignature);
        auto parameterIndex = mLastRootSignature->GetIndex(HAL::RootSignature::RegisterType::ShaderResource, registerIndex, registerSpace);
        if (mPassContext->GetQueueType() == CommandQueueType::Graphics)
        {
            mCommandList->SetGraphicsRootDescriptorTable(parameterIndex, descriptorHandle);
        }
        else
        {
            mCommandList->SetComputeRootDescriptorTable(parameterIndex, descriptorHandle);
        }
    }

    void PassCommandRecorder::IASetIndexBuffer(const Memory::IndexBuffer *indexBuffer)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
        indexBuffer->GetSRDescriptor();
    }

    void PassCommandRecorder::IASetVertexBuffers(const Memory::VertexBuffer* vertexBuffer)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
        vertexBuffer->GetSRDescriptor();
    }

    void PassCommandRecorder::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->IASetPrimitiveTopology(topology);
    }

    void PassCommandRecorder::Draw(uint32 vertexCount, uint32 vertexStart)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->DrawInstanced(vertexCount, 1, vertexStart, 0);
    }

    void PassCommandRecorder::DrawInstanced(uint32 vertexCount, uint32 vertexStart, uint32 instanceCount)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->DrawInstanced(vertexCount, instanceCount, vertexStart, 1);
    }

    void PassCommandRecorder::DrawIndexed(uint32 vertexStart, uint32 indexCount, uint32 indexStart)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->DrawIndexedInstanced(indexCount, 1, indexStart, vertexStart, 0);
    }

    void PassCommandRecorder::DrawIndexedInstanced(uint32 vertexStart, uint32 indexCount, uint32 indexStart, uint32 instanceCount)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Graphics);
        mCommandList->DrawIndexedInstanced(indexCount, instanceCount, indexStart, vertexStart, 1);
    }

    void PassCommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
    {
        assert(mPassContext->GetQueueType() == CommandQueueType::Compute);
        mCommandList->Dispatch(x, y, z);
    }

    void PassCommandRecorder::UAVBarrier(ID3D12Resource* res)
    {
        auto b = CD3DX12_RESOURCE_BARRIER::UAV(res);
        mCommandList->ResourceBarrier(1, &b);
    }

} // namespace Engine::Render