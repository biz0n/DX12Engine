#pragma once

#include "Types.h"
#include "Buffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "SceneLoader.h"
#include "CommandListContext.h"

#include <d3d12.h>
#include <vector>

namespace RenderUtils
{
    void UploadVertexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, VertexBuffer& vertexBuffer, CommandListContext& commandListContext);

    void UploadIndexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, IndexBuffer& indexBuffer, CommandListContext& commandListContext);

    void UploadBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Buffer& buffer, CommandListContext& commandListContext,  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void UploadTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture& texture, CommandListContext& commandListContext);

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const VertexBuffer& vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const IndexBuffer& indexBuffer);
}