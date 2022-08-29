#include "SceneObject.h"

#include <Scene/Components/CameraComponent.h>

namespace Engine::Scene
{
    SceneObject::SceneObject() = default;

    SceneObject::~SceneObject() = default;

    entt::registry& SceneObject::GetRegistry() 
    { 
        return registry; 
    }

    const entt::registry& SceneObject::GetRegistry() const
    {
        return registry;
    }

    const std::tuple<entt::entity, Scene::Components::CameraComponent> SceneObject::GetMainCamera() const
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

    void SceneObject::AddSystem(UniquePtr<Systems::System> system)
    {
        system->Init(this);
        mSystems.push_back(std::move(system));
    }

    void SceneObject::Process(const Timer &timer)
    {
        for (int i = 0; i < mSystems.size(); ++i)
        {
            mSystems[i]->Process(this, timer);
        }
    }
} // namespace Engine::Scene