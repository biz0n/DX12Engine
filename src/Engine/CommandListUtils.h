#pragma once

#include <Types.h>
#include <Buffer.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <ShaderTypes.h>
#include <RenderContext.h>

#include <Memory/UploadBuffer.h>
#include <Scene/SceneForwards.h>
#include <ResourceStateTracker.h>
#include <Memory/MemoryForwards.h>
#include <Scene/PunctualLight.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::CommandListUtils
{
    void UploadVertexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, VertexBuffer &vertexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void UploadIndexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, IndexBuffer &indexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void UploadBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, Buffer &buffer, SharedPtr<Engine::UploadBuffer> uploadBuffer, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void UploadTexture(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, Scene::Texture *texture, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    bool UploadMaterialTextures(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<Scene::Material> material, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, VertexBuffer &vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, IndexBuffer &indexBuffer);

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material);

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world);
    MaterialUniform GetMaterialUniform(const Scene::Material *material);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState);

    void ClearCache();
} // namespace Engine::RenderUtils