#pragma once

#include <Types.h>
#include <Render/ShaderTypes.h>

#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::CommandListUtils
{
    void BindVertexBuffer(PassCommandRecorder* commandRecorder, SharedPtr<Memory::ResourceStateTracker> stateTracker, const Memory::VertexBuffer* vertexBuffer);
    void BindIndexBuffer(PassCommandRecorder* commandRecorder, SharedPtr<Memory::ResourceStateTracker> stateTracker, const Memory::IndexBuffer* indexBuffer);

    D3D12_GPU_VIRTUAL_ADDRESS BindMaterial(SharedPtr<Memory::ResourceStateTracker> stateTracker, SharedPtr<Memory::UploadBuffer> buffer, SharedPtr<Scene::Material> material);

    Shader::LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world);
    Shader::MaterialUniform GetMaterialUniform(const Scene::Material& material);
    Shader::FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount);
    Shader::MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world);

    void TransitionBarrier(Memory::ResourceStateTracker* stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState);



    void ClearCache();
} // namespace Engine::RenderUtils