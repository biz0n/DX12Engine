#include "CubePassSystem.h"

#include <Render/Renderer.h>
#include <Render/Passes/CubePass.h>
#include <Render/Passes/Data/PassData.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/CubeMapComponent.h>

namespace Engine::Render::Systems
{
    CubePassSystem::CubePassSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(renderer)
    {
    }

    CubePassSystem::~CubePassSystem() = default;

    void CubePassSystem::Init(Scene::SceneObject *scene)
    {
        mCubePass = MakeUnique<Render::Passes::CubePass>();
    }

    void CubePassSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        auto &registry = scene->GetRegistry();

        Render::Passes::CubePassData data = {};

        auto view = registry.view<Scene::Components::CubeMapComponent>();
        if (!view.empty())
        {
            auto cameraEntity = registry.view<Scene::Components::CameraComponent>().front();
            auto camera = registry.get<Scene::Components::CameraComponent>(cameraEntity);

            data.camera.camera = camera.camera;
            data.camera.projection = camera.projection;
            data.camera.view = camera.view;
            data.camera.viewProjection = camera.viewProjection;
            data.camera.eyePosition = camera.eyePosition;

            data.hasCube = true;
            auto cubeComponent = registry.get<Scene::Components::CubeMapComponent>(view.front());
            data.cube.cubeMap = cubeComponent.cubeMap;

        }

        mCubePass->SetPassData(data);

        mRenderer->RegisterRenderPass(mCubePass.get());
    }
} // namespace Engine::Scene::Systems