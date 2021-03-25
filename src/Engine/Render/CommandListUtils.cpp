#include "CommandListUtils.h"

#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/UploadBuffer.h>
#include <Memory/Texture.h>

#include <Render/RenderContext.h>
#include <Render/ResourceStateTracker.h>

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
    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::VertexBuffer &vertexBuffer)
    {
        TransitionBarrier(stateTracker, vertexBuffer.D3DResource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        auto vbv = vertexBuffer.GetVertexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbv);
    }

    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Memory::IndexBuffer &indexBuffer)
    {
        TransitionBarrier(stateTracker, indexBuffer.D3DResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

        auto ibv = indexBuffer.GetIndexBufferView();
        commandList->IASetIndexBuffer(&ibv);
    }

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world)
    {
        LightUniform light = {};

        light.LightType = lightNode.GetLightType();
        light.Enabled = lightNode.IsEnabled();
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

    MaterialUniform GetMaterialUniform(const Scene::Material& material)
    {
        Scene::MaterialProperties properties = material.GetProperties();
        MaterialUniform uniform = {};
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

        return uniform;
    }

    FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount)
    {
        FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, viewProj);

        dx::XMStoreFloat3(&cb.EyePos, eyePos);

        cb.LightsCount = lightsCount;

        return cb;
    }

    MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world)
    {
        DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(world);
        auto d = DirectX::XMMatrixDeterminant(tWorld);
        DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
        tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);
        MeshUniform cb;
        DirectX::XMStoreFloat4x4(&cb.World, tWorld);
        DirectX::XMStoreFloat4x4(&cb.InverseTranspose, tWorldInverseTranspose);

        return cb;
    }

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Memory::UploadBuffer> buffer, SharedPtr<Memory::DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material)
    {
        auto device = renderContext->Device();
        auto descriptorAllocator = renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        MaterialUniform uniform = CommandListUtils::GetMaterialUniform(*material.get());

        auto matAllocation = buffer->Allocate(sizeof(MaterialUniform));
        matAllocation.CopyTo(&uniform);
        commandList->SetGraphicsRootConstantBufferView(2, matAllocation.GPU);

        if (material->HasBaseColorTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetBaseColorTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(4, 0, 1, material->GetBaseColorTexture()->GetSRDescriptor().GetCPUDescriptor());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetMetallicRoughnessTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(5, 0, 1, material->GetMetallicRoughnessTexture()->GetSRDescriptor().GetCPUDescriptor());
        }

        if (material->HasNormalTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetNormalTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(6, 0, 1, material->GetNormalTexture()->GetSRDescriptor().GetCPUDescriptor());
        }

        
        if (material->HasEmissiveTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetEmissiveTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(7, 0, 1, material->GetEmissiveTexture()->GetSRDescriptor().GetCPUDescriptor());
        }

        if (material->HasAmbientOcclusionTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetAmbientOcclusionTexture()->D3DResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(8, 0, 1, material->GetAmbientOcclusionTexture()->GetSRDescriptor().GetCPUDescriptor());
        }
    }

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush)
    {
        TransitionBarrier(stateTracker, resource, targetState);

        if (forceFlush)
        {
            stateTracker->FlushBarriers(commandList);
        }
    }

    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            targetState);
        stateTracker->ResourceBarrier(barrier);
    }

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState, bool forceFlush)
    {
        TransitionBarrier(stateTracker, resource, targetState);

        if (forceFlush)
        {
            stateTracker->FlushBarriers(commandList);
        }
    }

    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                resource,
                D3D12_RESOURCE_STATE_COMMON,
                targetState);
        stateTracker->ResourceBarrier(barrier);
    }

} // namespace Engine::CommandListUtils