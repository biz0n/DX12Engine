#pragma once

#include <Types.h>
#include <Buffer.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <ShaderTypes.h>

#include <Scene/SceneForwards.h>
#include <CommandListContext.h>
#include <ResourceStateTracker.h>
#include <UploadBuffer.h>
#include <DynamicDescriptorHeap.h>

#include <d3d12.h>
#include <vector>

namespace Engine::CommandListUtils
{
    void UploadVertexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, VertexBuffer &vertexBuffer, CommandListContext &commandListContext);

    void UploadIndexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, IndexBuffer &indexBuffer, CommandListContext &commandListContext);

    void UploadBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Buffer &buffer, CommandListContext &commandListContext, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void UploadTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList,  SharedPtr<GlobalResourceStateTracker> resourceTracker, Scene::Texture &texture, CommandListContext &commandListContext);

    void UploadMaterialTextures(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<GlobalResourceStateTracker> resourceTracker, CommandListContext &commandListContext, SharedPtr<Scene::Material> material);

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const VertexBuffer &vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const IndexBuffer &indexBuffer);

    void BindMaterial(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material);

    LightUniform GetLightUniform(const Scene::LightNode *lightNode);
    MaterialUniform GetMaterialUniform(const Scene::Material *material);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState);
} // namespace Engine::RenderUtils