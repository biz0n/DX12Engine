#pragma once

#include <Types.h>
#include <ShaderTypes.h>

#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::CommandListUtils
{
    void UploadVertexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::VertexBuffer &vertexBuffer, SharedPtr<Memory::UploadBuffer> uploadBuffer);

    void UploadIndexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::IndexBuffer &indexBuffer, SharedPtr<Memory::UploadBuffer> uploadBuffer);

    void UploadBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::Buffer &buffer, SharedPtr<Memory::UploadBuffer> uploadBuffer, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void UploadTexture(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Scene::Texture *texture, SharedPtr<Memory::UploadBuffer> uploadBuffer);

    bool UploadMaterialTextures(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Scene::Material> material, SharedPtr<Memory::UploadBuffer> uploadBuffer);

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::VertexBuffer &vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::IndexBuffer &indexBuffer);

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Memory::UploadBuffer> buffer, SharedPtr<Memory::DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material);

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world);
    MaterialUniform GetMaterialUniform(const Scene::Material& material);
    FrameUniform GetFrameUniform(const Scene::Camera& camera, const DirectX::XMMATRIX& world, float32 aspectRatio, uint32 lightsCount);
    MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState);

    void ClearCache();
} // namespace Engine::RenderUtils