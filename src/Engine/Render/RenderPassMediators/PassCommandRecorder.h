#pragma once

#include <Types.h>
#include <Name.h>

#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>
#include <HAL/HALForwards.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{
    class PassCommandRecorder
    {
    public:
        PassCommandRecorder(
            PassContext* passContext,
            ComPtr<ID3D12GraphicsCommandList> commandList,
            Memory::ResourceStateTracker *resourceStateTracker,
            RenderContext *renderContext,
            const FrameResourceProvider *frameResourceProvider,
            FrameTransientContext* frameTransientContext);
        ~PassCommandRecorder();

        ComPtr<ID3D12GraphicsCommandList> GetD3D12CommandList() const { return mCommandList; }

        void SetViewPort(uint32 width = 0, uint32 height = 0);

        void SetRenderTargets(const std::vector<Name>& renderTargets, const Name& depthStencil);
        void SetBackBufferAsRenderTarget();

        void ClearRenderTargets(const std::vector<Name>& renderTargets);
        void ClearDepthStencil(const Name& depthStencil);

        void SetPipelineState(const Name& pso);

        void SetRoot32BitConstants(uint32 registerIndex, uint32 registerSpace, uint32 num32BitValuesToSet, const void *srcData, uint32 destOffsetIn32BitValues);
        void SetRootConstantBufferView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetRootShaderResourceView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetRootUnorderedAccessView(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
        void SetRootDescriptorTable(uint32 registerIndex, uint32 registerSpace, D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle);

        void IASetVertexBuffers(const Memory::VertexBuffer* vertexBuffer);
        void IASetIndexBuffer(const Memory::IndexBuffer* indexBuffer);
        void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

        void Draw(uint32 vertexCount, uint32 vertexStart);
        void DrawInstanced(uint32 vertexCount, uint32 vertexStart, uint32 instanceCount);
        void DrawIndexed(uint32 vertexStart, uint32 indexCount, uint32 indexStart);
        void DrawIndexedInstanced(uint32 vertexStart, uint32 indexCount, uint32 indexStart, uint32 instanceCount);

        void Dispatch(uint32 x, uint32 y, uint32 z = 1);

        void UAVBarrier(ID3D12Resource* res);
    private:
        void SetRootSignature(const Name& rootSignature);
    private:
        PassContext* mPassContext;
        ComPtr<ID3D12GraphicsCommandList> mCommandList;
        Memory::ResourceStateTracker *mResourceStateTracker;
        RenderContext *mRenderContext;
        const FrameResourceProvider *mFrameResourceProvider;
        FrameTransientContext* mFrameTransientContext;

        Name mLastRootSignatureName;
        Name mLastPSOName;
        HAL::RootSignature* mLastRootSignature;
    };

} // namespace Engine::Render
