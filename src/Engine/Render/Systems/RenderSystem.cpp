#include "RenderSystem.h"


#include <RenderContext.h>
#include <Game.h>

#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>

#include <Render/RenderingScene.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    RenderSystem::RenderSystem(SharedPtr<RenderContext> renderContext)
     : mRenderContext(renderContext)
     {
     }
    
    RenderSystem::~RenderSystem() = default;

    void RenderSystem::Init(entt::registry *registry)
    {
        mGame = MakeUnique<Game>(mRenderContext);
        mGame->Initialize();
    }

    void RenderSystem::Process(entt::registry *registry, const Timer& timer)
    {   
        mGame->UploadResources(registry);
        mGame->Draw(timer, registry);


        /*
        Render::RenderingScene scene;

        const auto& meshes = registry->view<Components::MeshComponent, Components::WorldTransformComponent>();
        const auto& lights = registry->view<Components::LightComponent, Components::WorldTransformComponent>();
        const auto& cameras = registry->view<Components::CameraComponent, Components::WorldTransformComponent>();

        scene.meshes.reserve(meshes.size());
        scene.lights.reserve(lights.size());
        scene.cameras.reserve(cameras.size());

        for (auto &&[entity, meshComponent, transformComponent] : meshes.proxy())
        {
            Render::Mesh mesh;
            mesh.mesh = meshComponent.mesh;
            mesh.worldTransform = transformComponent.transform;

            scene.meshes.push_back(mesh);
        }

        for (auto &&[entity, lightComponent, transformComponent] : lights.proxy())
        {
            if (lightComponent.light.IsEnabled())
            {
                Render::Light light;
                light.light = lightComponent.light;
                light.worldTransform = transformComponent.transform;

                scene.lights.push_back(light);
            }
        }

        for (auto &&[entity, cameraComponent, transformComponent] : cameras.proxy())
        {
            Render::Camera camera;
            camera.camera = cameraComponent.camera;
            camera.worldTransform = transformComponent.transform;

            scene.cameras.push_back(camera);
        }
        */
    }
}