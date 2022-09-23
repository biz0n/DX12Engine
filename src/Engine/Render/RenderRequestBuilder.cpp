#include "RenderRequestBuilder.h"

#include <Scene/SceneObject.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>

#include <Bin3D/Material.h>

#include <Scene/MeshResources.h>
#include <Memory/VertexBuffer.h>
#include <Memory/IndexBuffer.h>

#include <entt/entt.hpp>

namespace Engine::Render
{
    RenderRequest RenderRequestBuilder::BuildRequest(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage)
    {
        RenderRequest renderRequest{
            std::move(PrepareMeshes(sceneObject, sceneStorage)),
            std::move(PrepareLights(sceneObject, sceneStorage)),
            std::move(PrepareMaterials(sceneObject, sceneStorage)),
            std::move(PrepareRenderCamera(sceneObject)),
            std::move(PrepareShadowCameras(sceneObject)),
            sceneStorage
        };

        return renderRequest;
    }

    RenderCamera RenderRequestBuilder::PrepareRenderCamera(const Engine::Scene::SceneObject* sceneObject)
    {
        const auto& [cameraEntity, camera] = sceneObject->GetMainCamera();

        RenderCamera renderCamera{};
        renderCamera.projection = camera.projection;
        renderCamera.view = camera.view;
        renderCamera.viewProjection = camera.viewProjection;
        renderCamera.eyePosition = camera.eyePosition;

        return renderCamera;
    }

    MeshPack RenderRequestBuilder::PrepareMeshes(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage)
    {
        auto& registry = sceneObject->GetRegistry();

        auto& [cameraEntity, camera] = sceneObject->GetMainCamera();

        std::vector<Shader::MeshUniform> meshes;

        const auto& meshsView = registry.view<
            Scene::Components::MeshComponent,
            Scene::Components::WorldTransformComponent,
            Scene::Components::AABBComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);
        meshes.reserve(meshsView.size_hint());

        for (auto&& [entity, meshComponent, transformComponent, aabbComponent] : meshsView.each())
        {
            if (!camera.frustum.Intersects(aabbComponent.boundingBox))
            {
               // continue;
            }

            Shader::MeshUniform meshUniform = {};
            const auto& mesh = sceneStorage->GetMeshes()[meshComponent.MeshIndex];

            meshUniform.Id = meshComponent.MeshIndex;
            meshUniform.MaterialIndex = meshComponent.MaterialIndex;
            meshUniform.VertexBufferIndex = mesh.vertexCoordinatesBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.VertexPropertiesBufferIndex = mesh.vertexPropertiesBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.IndexBufferIndex = mesh.indexBuffer->GetSRDescriptor().GetFullIndex();

            meshUniform.MeshletBufferIndex = mesh.meshletsBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.PrimitiveIndexBufferIndex = mesh.primitiveIndicesBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.UniqueVertexIndexBufferIndex = mesh.uniqueVertexIndexBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.MeshletCount = mesh.GetMeshletsCount();
            

            DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(transformComponent.transform);
            auto d = DirectX::XMMatrixDeterminant(tWorld);
            DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
            tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);

            DirectX::XMStoreFloat4x4(&meshUniform.World, tWorld);
            DirectX::XMStoreFloat4x4(&meshUniform.InverseTranspose, tWorldInverseTranspose);

            meshes.push_back(meshUniform);
        }

        MeshPack pack;
        pack.meshes = std::move(meshes);
        pack.opaque = { 0, pack.meshes.size() };
        return pack;
    }

    std::vector<Shader::MaterialUniform> RenderRequestBuilder::PrepareMaterials(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage)
    {
        std::vector<Shader::MaterialUniform> materials;

        for (const auto& material : sceneStorage->GetMaterials())
        {
            Shader::MaterialUniform uniform = {};

            const auto& properties = material.MaterialProperties;
            uniform.BaseColor = properties.baseColor.baseColor;
            uniform.EmissiveFactor = { properties.emissive.factor.x, properties.emissive.factor.y, properties.emissive.factor.z, 1.0f };
            uniform.MetallicFactor = properties.metallicRaughness.metallicFactor;
            uniform.RoughnessFactor = properties.metallicRaughness.roughnessFactor;
            uniform.NormalScale = properties.normalTextureInfo.scale;
            uniform.Ambient = { 0.9f, 0.9f, 0.9f, 0.0f };
            uniform.Cutoff = properties.alphaCutoff;

            uniform.HasBaseColorTexture = sceneStorage->HasTexture(material.BaseColorTextureIndex);
            uniform.HasNormalTexture = sceneStorage->HasTexture(material.NormalTextureIndex);
            uniform.HasMetallicRoughnessTexture = sceneStorage->HasTexture(material.MetallicRoughnessTextureIndex);
            uniform.HasOcclusionTexture = sceneStorage->HasTexture(material.AmbientOcclusionTextureIndex);
            uniform.HasEmissiveTexture = sceneStorage->HasTexture(material.EmissiveTextureIndex);

            if (uniform.HasBaseColorTexture)
            {
                uniform.BaseColorIndex = sceneStorage->GetTexture(material.BaseColorTextureIndex)->GetSRDescriptor().GetFullIndex();
            }

            if (uniform.HasMetallicRoughnessTexture)
            {
                uniform.MetallicRoughnessIndex = sceneStorage->GetTexture(material.MetallicRoughnessTextureIndex)->GetSRDescriptor().GetFullIndex();
            }

            if (uniform.HasNormalTexture)
            {
                uniform.NormalIndex = sceneStorage->GetTexture(material.NormalTextureIndex)->GetSRDescriptor().GetFullIndex();
            }

            if (uniform.HasEmissiveTexture)
            {
                uniform.EmissiveIndex = sceneStorage->GetTexture(material.EmissiveTextureIndex)->GetSRDescriptor().GetFullIndex();
            }

            if (uniform.HasOcclusionTexture)
            {
                uniform.OcclusionIndex = sceneStorage->GetTexture(material.AmbientOcclusionTextureIndex)->GetSRDescriptor().GetFullIndex();
            }

            materials.push_back(uniform);
        }

        return materials;
    }

    std::vector<Shader::LightUniform> RenderRequestBuilder::PrepareLights(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage)
    {
        auto& registry = sceneObject->GetRegistry();

        const auto& lightsView = registry.view<Scene::Components::LightComponent, Scene::Components::WorldTransformComponent>();

        std::vector<Shader::LightUniform> lights;

        for (const auto&& [entity, lightComponent, transformComponent] : lightsView.each())
        {
            Shader::LightUniform uniform = {};

            const auto& lightNode = lightComponent.light;
            const auto& world = transformComponent.transform;

            uniform.LightType = (int)lightNode.LightType;
            auto color = lightNode.Color;
            uniform.Color = { color.x * lightNode.Intensity, color.y * lightNode.Intensity, color.z * lightNode.Intensity };

            DirectX::XMStoreFloat3(&uniform.PositionWS,
                DirectX::XMVector4Transform(
                    DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                    world));

            DirectX::XMStoreFloat3(&uniform.DirectionWS,
                DirectX::XMVector4Transform(
                    DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.0f),
                    world));

            uniform.ConstantAttenuation = lightNode.ConstantAttenuation;
            uniform.LinearAttenuation = lightNode.LinearAttenuation;
            uniform.QuadraticAttenuation = lightNode.QuadraticAttenuation;
            uniform.InnerConeAngle = lightNode.InnerConeAngle;
            uniform.OuterConeAngle = lightNode.OuterConeAngle;

            lights.push_back(uniform);
        }

        return lights;
    }

    std::vector<RenderCamera> RenderRequestBuilder::PrepareShadowCameras(const Engine::Scene::SceneObject* sceneObject)
    {
        auto& registry = sceneObject->GetRegistry();

        std::vector<RenderCamera> renderCameras;

        auto cameras = registry.view<Scene::Components::CameraComponent, Scene::Components::LightComponent>();

        for (auto&& [entity, cameraComponent, lightComponent] : cameras.each())
        {
            if (lightComponent.light.LightType == Bin3D::LightType::DirectionalLight)
            {
                RenderCamera renderCamera{};
                renderCamera.projection = cameraComponent.projection;
                renderCamera.view = cameraComponent.view;
                renderCamera.viewProjection = cameraComponent.viewProjection;
                renderCamera.eyePosition = cameraComponent.eyePosition;

                renderCameras.push_back(renderCamera);

                break; //now supports only one shadow camera
            }
        }

        return renderCameras;
    }
}