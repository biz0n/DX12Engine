#include "ForwardPassSystem.h"

#include <Render/Renderer.h>
#include <Render/Passes/ForwardPass.h>
#include <Render/Passes/Data/PassData.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/IsDisabledComponent.h>

namespace Engine::Render::Systems
{
    ForwardPassSystem::ForwardPassSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(renderer)
    {
    }

    ForwardPassSystem::~ForwardPassSystem() = default;

    void ForwardPassSystem::Init(Scene::SceneObject *scene)
    {
        mForwardPass = MakeUnique<Render::Passes::ForwardPass>();
    }

    void ForwardPassSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        auto &registry = scene->GetRegistry();

        Render::Passes::ForwardPassData data = {};

        auto [cameraEntity, camera] = scene->GetMainCamera();

        data.camera.projection = camera.projection;
        data.camera.view = camera.view;
        data.camera.viewProjection = camera.viewProjection;
        data.camera.eyePosition = camera.eyePosition;

        auto shadowCameraComponent = camera;
        auto cameras = registry.view<Scene::Components::CameraComponent, Scene::Components::LightComponent>();

        for (auto&& [entity, cameraComponent, lightComponent] : cameras.each())
        {
            if (lightComponent.light.GetLightType() == Scene::LightType::DirectionalLight)
            {
                shadowCameraComponent = cameraComponent;
                break;
            }
        }

        const dx::XMMATRIX T(
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f);

        dx::XMStoreFloat4x4(&data.shadowTransform, dx::XMMatrixTranspose(dx::XMMatrixTranspose(shadowCameraComponent.viewProjection) * T));

        const auto &lightsView = registry.view<Scene::Components::LightComponent, Scene::Components::WorldTransformComponent>();
        
        data.lights.reserve(lightsView.size_hint());
        for (auto &&[entity, lightComponent, transformComponent] : lightsView.each())
        {
            if (lightComponent.light.IsEnabled())
            {
                Render::Passes::LightData lightData = {};
                lightData.light = lightComponent.light;
                lightData.worldTransform = transformComponent.transform;
                data.lights.emplace_back(lightData);
            }
        }

        const auto &meshsView = registry.view<
            Scene::Components::MeshComponent, 
            Scene::Components::WorldTransformComponent, 
            Scene::Components::AABBComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);
        data.meshes.reserve(meshsView.size_hint());
        for (auto &&[entity, meshComponent, transformComponent, aabbComponent] : meshsView.each())
        {
            if (camera.frustum.Intersects(aabbComponent.boundingBox))
            {
                Render::Passes::MeshData meshData = {};
                meshData.mesh = meshComponent.mesh;
                meshData.worldTransform = transformComponent.transform;

                data.meshes.push_back(meshData);
            }
        }

        mForwardPass->SetPassData(data);

        mRenderer->RegisterRenderPass(mForwardPass.get());
    }
} // namespace Engine::Scene::Systems