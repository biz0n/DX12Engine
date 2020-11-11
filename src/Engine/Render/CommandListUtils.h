#pragma once

#include <Types.h>
#include <Memory/Buffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/IndexBuffer.h>
#include <ShaderTypes.h>
#include <Render/RenderContext.h>

#include <Memory/UploadBuffer.h>
#include <Scene/SceneForwards.h>
#include <Render/ResourceStateTracker.h>
#include <Memory/MemoryForwards.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::CommandListUtils
{
    void UploadVertexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, VertexBuffer &vertexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void UploadIndexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, IndexBuffer &indexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void UploadBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Buffer &buffer, SharedPtr<Engine::UploadBuffer> uploadBuffer, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void UploadTexture(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Scene::Texture *texture, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    bool UploadMaterialTextures(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Scene::Material> material, SharedPtr<Engine::UploadBuffer> uploadBuffer);

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, VertexBuffer &vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, IndexBuffer &indexBuffer);

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material);

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world);
    MaterialUniform GetMaterialUniform(const Scene::Material& material);
    FrameUniform GetFrameUniform(const Scene::Camera& camera, const DirectX::XMMATRIX& world, float32 aspectRatio, uint32 lightsCount);
    MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState);

    void ClearCache();
} // namespace Engine::RenderUtils