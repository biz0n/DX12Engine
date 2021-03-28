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
    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::VertexBuffer &vertexBuffer);
    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::IndexBuffer &indexBuffer);

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Memory::UploadBuffer> buffer, SharedPtr<Scene::Material> material);

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world);
    MaterialUniform GetMaterialUniform(const Scene::Material& material);
    FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount);
    MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState);

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState, bool forceFlush = false);
    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState);

    void ClearCache();
} // namespace Engine::RenderUtils