#include "SceneStorage.h"

#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Memory/VertexBuffer.h>
#include <Memory/IndexBuffer.h>

#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/PunctualLight.h>

#include <Render/Shaders/ShaderTypes.h>

#include <Memory/UploadBuffer.h>

#include <vector>

namespace Engine::Scene
{
    void SceneStorage::UploadSceneStructures(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        UploadMeshes(registry, uploadBuffer);
        UploadMaterials(uploadBuffer);
        UploadLights(registry, uploadBuffer);
    }

    void SceneStorage::UploadMeshes(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        const auto& meshsView = registry.view<Scene::Components::MeshComponent, Scene::Components::WorldTransformComponent>();

        mMeshUniforms.clear();
        mMeshUniforms.assign(mMeshes.size(), {});

        for (const auto&& [entity, meshComponent, transformComponent] : meshsView.each())
        {
            auto& meshUniform = mMeshUniforms[meshComponent.MeshIndex];
            const auto& mesh = mMeshes[meshComponent.MeshIndex];

            meshUniform.MaterialIndex = meshComponent.MaterialIndex;
            meshUniform.VertexBufferIndex = mesh.vertexBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.IndexBufferIndex = mesh.indexBuffer->GetSRDescriptor().GetFullIndex();

            DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(transformComponent.transform);
            auto d = DirectX::XMMatrixDeterminant(tWorld);
            DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
            tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);

            DirectX::XMStoreFloat4x4(&meshUniform.World, tWorld);
            DirectX::XMStoreFloat4x4(&meshUniform.InverseTranspose, tWorldInverseTranspose);
        }

        mMeshUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::MeshUniform) * mMeshUniforms.size());
        mMeshUniformsAllocation.CopyTo(mMeshUniforms);
    }

    void SceneStorage::UploadMaterials(SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        mMaterialUniforms.clear();

        for (const auto& material : mMaterials)
        {
            Shader::MaterialUniform uniform = {};

            const auto& properties = material.GetProperties();
            uniform.BaseColor = properties.baseColor.baseColor;
            uniform.EmissiveFactor = { properties.emissive.factor.x, properties.emissive.factor.y, properties.emissive.factor.z, 1.0f };
            uniform.MetallicFactor = properties.metallicRaughness.metallicFactor;
            uniform.RoughnessFactor = properties.metallicRaughness.roughnessFactor;
            uniform.NormalScale = properties.normalTextureInfo.scale;
            uniform.Ambient = { 0.9f, 0.9f, 0.9f, 0.0f };
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

            mMaterialUniforms.push_back(uniform);
        }

        mMaterialUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::MaterialUniform) * mMaterialUniforms.size());
        mMaterialUniformsAllocation.CopyTo(mMaterialUniforms);
    }

    void SceneStorage::UploadLights(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        const auto& lightsView = registry.view<Scene::Components::LightComponent, Scene::Components::WorldTransformComponent>();

        mLightUniforms.clear();
        mHasDirectionalLight = false;

        for (const auto&& [entity, lightComponent, transformComponent] : lightsView.each())
        {
            if (!lightComponent.light.IsEnabled())
            {
                continue;
            }

            Shader::LightUniform uniform = {};

            const auto& lightNode = lightComponent.light;
            const auto& world = transformComponent.transform;

            uniform.LightType = lightNode.GetLightType();
            auto color = lightNode.GetColor();
            uniform.Color = { color.x * lightNode.GetIntensity(), color.y * lightNode.GetIntensity(), color.z * lightNode.GetIntensity() };

            DirectX::XMStoreFloat3(&uniform.PositionWS,
                DirectX::XMVector4Transform(
                    DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                    world));

            DirectX::XMFLOAT3 direction = lightNode.GetDirection();
            DirectX::XMStoreFloat3(&uniform.DirectionWS,
                DirectX::XMVector4Transform(
                    DirectX::XMVectorSet(direction.x, direction.y, direction.z, 0.0f),
                    world));

            uniform.ConstantAttenuation = lightNode.GetConstantAttenuation();
            uniform.LinearAttenuation = lightNode.GetLinearAttenuation();
            uniform.QuadraticAttenuation = lightNode.GetQuadraticAttenuation();
            uniform.InnerConeAngle = lightNode.GetInnerConeAngle();
            uniform.OuterConeAngle = lightNode.GetOuterConeAngle();

            mLightUniforms.push_back(uniform);

            if (lightNode.GetLightType() == LightType::DirectionalLight)
            {
                mHasDirectionalLight = true;
            }
        }

        mLightUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::LightUniform) * mLightUniforms.size());
        mLightUniformsAllocation.CopyTo(mLightUniforms);
        
    }
}