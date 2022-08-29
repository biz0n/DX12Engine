#include "RenderRequestBuilder.h"

#include <Scene/SceneObject.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>
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
                continue;
            }

            Shader::MeshUniform meshUniform = {};
            const auto& mesh = sceneStorage->GetMeshes()[meshComponent.MeshIndex];

            meshUniform.Id = meshComponent.MeshIndex;
            meshUniform.MaterialIndex = meshComponent.MaterialIndex;
            meshUniform.VertexBufferIndex = mesh.vertexBuffer->GetSRDescriptor().GetFullIndex();
            meshUniform.IndexBufferIndex = mesh.indexBuffer->GetSRDescriptor().GetFullIndex();

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

            DirectX::XMStoreFloat3(&uniform.DirectionWS,
                DirectX::XMVector4Transform(
                    DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.0f),
                    world));

            uniform.ConstantAttenuation = lightNode.GetConstantAttenuation();
            uniform.LinearAttenuation = lightNode.GetLinearAttenuation();
            uniform.QuadraticAttenuation = lightNode.GetQuadraticAttenuation();
            uniform.InnerConeAngle = lightNode.GetInnerConeAngle();
            uniform.OuterConeAngle = lightNode.GetOuterConeAngle();

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
            if (lightComponent.light.GetLightType() == Scene::LightType::DirectionalLight)
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