#include "DepthPassSystem.h"

#include <Render/Renderer.h>
#include <Render/Passes/DepthPass.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/IsDisabledComponent.h>

namespace Engine::Render::Systems
{
    DepthPassSystem::DepthPassSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(renderer)
    {
    }

    DepthPassSystem::~DepthPassSystem() = default;

    void DepthPassSystem::Init(Scene::SceneObject *scene)
    {
        mDepthPass = MakeUnique<Render::Passes::DepthPass>();
    }

    void DepthPassSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        auto &registry = scene->GetRegistry();

        Render::Passes::DepthPassData data = {};

        auto [cameraEntity, camera] = scene->GetMainCamera();

        auto cameras = registry.view<Scene::Components::CameraComponent, Scene::Components::LightComponent>();

        for (auto&& [entity, cameraComponent, lightComponent] : cameras.each())
        {
            if (lightComponent.light.GetLightType() == Scene::LightType::DirectionalLight)
            {
                camera = cameraComponent;
                cameraEntity = entity;
                break;
            }
        }

        data.camera.projection = camera.projection;
        data.camera.view = camera.view;
        data.camera.viewProjection = camera.viewProjection;
        data.camera.eyePosition = camera.eyePosition;

        const auto &meshsView = registry.view<
            Scene::Components::MeshComponent, 
            Scene::Components::AABBComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);
        data.meshes.reserve(meshsView.size_hint());
        for (auto &&[entity, meshComponent, aabbComponent] : meshsView.each())
        {
            if (camera.frustum.Intersects(aabbComponent.boundingBox))
            {
                Render::Passes::MeshData meshData = {meshComponent.MeshIndex};

                data.meshes.push_back(meshData);
            }
        }
        mDepthPass->SetPassData(data);

        mRenderer->RegisterRenderPass(mDepthPass.get());
    }
} // namespace Engine::Scene::Systems