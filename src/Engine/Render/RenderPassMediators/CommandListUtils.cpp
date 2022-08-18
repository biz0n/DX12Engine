#include "CommandListUtils.h"

#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/UploadBuffer.h>
#include <Memory/Texture.h>

#include <Render/RenderContext.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>

#include <Scene/Loader/SceneLoader.h>
#include <Scene/Image.h>
#include <Scene/Material.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>

#include <DirectXTex.h>
#include <DirectXMath.h>

#include <filesystem>
#include <map>

namespace Engine::Render::CommandListUtils
{
    void BindVertexBuffer(PassCommandRecorder* commandRecorder, SharedPtr<Memory::ResourceStateTracker> stateTracker, const Memory::VertexBuffer* vertexBuffer)
    {
        TransitionBarrier(stateTracker.get(), vertexBuffer->D3DResource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        commandRecorder->IASetVertexBuffers(vertexBuffer);
    }

    void BindIndexBuffer(PassCommandRecorder* commandRecorder, SharedPtr<Memory::ResourceStateTracker> stateTracker, const Memory::IndexBuffer* indexBuffer)
    {
        TransitionBarrier(stateTracker.get(), indexBuffer->D3DResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

        commandRecorder->IASetIndexBuffer(indexBuffer);
    }

    Shader::LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world)
    {
        Shader::LightUniform light = {};

        light.LightType = lightNode.GetLightType();
        auto color = lightNode.GetColor();
        light.Color = { color.x * lightNode.GetIntensity(), color.y * lightNode.GetIntensity(), color.z * lightNode.GetIntensity() };

        DirectX::XMStoreFloat3(&light.PositionWS,
                               DirectX::XMVector4Transform(
                               DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                               world));

        DirectX::XMFLOAT3 direction = lightNode.GetDirection();
        DirectX::XMStoreFloat3(&light.DirectionWS,
                               DirectX::XMVector4Transform(
                               DirectX::XMVectorSet(direction.x, direction.y, direction.z, 0.0f),
                               world));

        light.ConstantAttenuation = lightNode.GetConstantAttenuation();
        light.LinearAttenuation = lightNode.GetLinearAttenuation();
        light.QuadraticAttenuation = lightNode.GetQuadraticAttenuation();
        light.InnerConeAngle = lightNode.GetInnerConeAngle();
        light.OuterConeAngle = lightNode.GetOuterConeAngle();

        return light;
    }

    Shader::MaterialUniform GetMaterialUniform(const Scene::Material& material)
    {
        Scene::MaterialProperties properties = material.GetProperties();
        Shader::MaterialUniform uniform = {};
        uniform.BaseColor = properties.baseColor.baseColor;
        uniform.EmissiveFactor = {properties.emissive.factor.x, properties.emissive.factor.y, properties.emissive.factor.z, 1.0f};
        uniform.MetallicFactor = properties.metallicRaughness.metallicFactor;
        uniform.RoughnessFactor = properties.metallicRaughness.roughnessFactor;
        uniform.NormalScale = properties.normalTextureInfo.scale;
        uniform.Ambient = {0.9f, 0.9f, 0.9f, 0.0f};
        uniform.Cutoff = properties.alphaCutoff;

        uniform.HasBaseColorTexture = material.HasBaseColorTexture();
        uniform.HasNormalTexture = material.HasNormalTexture();
        uniform.HasMetallicRoughnessTexture = material.HasMetallicRoughnessTexture();
        uniform.HasOcclusionTexture = material.HasAmbientOcclusionTexture();
        uniform.HasEmissiveTexture = material.HasEmissiveTexture();

        if (material.HasBaseColorTexture())
        {
            uniform.BaseColorIndex = material.GetBaseColorTexture()->GetSRDescriptor().GetFullIndex();
        }

        if (material.HasMetallicRoughnessTexture())
        {
            uniform.MetallicRoughnessIndex = material.GetMetallicRoughnessTexture()->GetSRDescriptor().GetFullIndex();
        }

        if (material.HasNormalTexture())
        {
            uniform.NormalIndex = material.GetNormalTexture()->GetSRDescriptor().GetFullIndex();
        }

        if (material.HasEmissiveTexture())
        {
            uniform.EmissiveIndex = material.GetEmissiveTexture()->GetSRDescriptor().GetFullIndex();
        }

        if (material.HasAmbientOcclusionTexture())
        {
            uniform.OcclusionIndex = material.GetAmbientOcclusionTexture()->GetSRDescriptor().GetFullIndex();
        }

        return uniform;
    }

    Shader::FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount)
    {
        Shader::FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, viewProj);

        dx::XMStoreFloat3(&cb.EyePos, eyePos);

        cb.LightsCount = lightsCount;

        return cb;
    }

    Shader::MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world)
    {
        DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(world);
        auto d = DirectX::XMMatrixDeterminant(tWorld);
        DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
        tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);
        Shader::MeshUniform cb{};
        DirectX::XMStoreFloat4x4(&cb.World, tWorld);
        DirectX::XMStoreFloat4x4(&cb.InverseTranspose, tWorldInverseTranspose);

        return cb;
    }

    D3D12_GPU_VIRTUAL_ADDRESS BindMaterial(SharedPtr<Memory::ResourceStateTracker> stateTracker, SharedPtr<Memory::UploadBuffer> buffer, SharedPtr<Scene::Material> material)
    {
        Shader::MaterialUniform uniform = CommandListUtils::GetMaterialUniform(*material.get());

        auto matAllocation = buffer->Allocate(sizeof(Shader::MaterialUniform));
        matAllocation.CopyTo(&uniform);
        return matAllocation.GPU;
    }

    void TransitionBarrier(Memory::ResourceStateTracker* stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource,
            D3D12_RESOURCE_STATE_COMMON,
            targetState);
        stateTracker->ResourceBarrier(barrier);
    }
} // namespace Engine::CommandListUtils