#include "SceneRegistry.h"

#include <Scene/Components/CameraComponent.h>

namespace Engine::Scene
{
    SceneRegistry::SceneRegistry() = default;

    SceneRegistry::~SceneRegistry() = default;

    entt::registry& SceneRegistry::GetRegistry()
    { 
        return registry; 
    }

    const entt::registry& SceneRegistry::GetRegistry() const
    {
        return registry;
    }

    const std::tuple<entt::entity, Scene::Components::CameraComponent> SceneRegistry::GetMainCamera() const
    {
        entt::entity cameraEntity = entt::null;
        Scene::Components::CameraComponent component{};

        for (const auto &&[e, c] : registry.view<Scene::Components::CameraComponent, Scene::Components::MainCameraComponent>().each())
        {
            cameraEntity = e;
            component = c;
            break;
        }
        return {cameraEntity, component};
    }

    void SceneRegistry::AddSystem(UniquePtr<Systems::System> system)
    {
        system->Init(this);
        mSystems.push_back(std::move(system));
    }

    void SceneRegistry::Process(const Timer &timer)
    {
        for (int i = 0; i < mSystems.size(); ++i)
        {
            mSystems[i]->Process(this, timer);
        }
    }
} // namespace Engine::Scene