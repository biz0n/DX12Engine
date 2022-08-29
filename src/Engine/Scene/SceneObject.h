#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>
#include <Scene/Components/ComponentsForwards.h>

#include <Scene/Systems/System.h>
#include <vector>
#include <tuple>
#include <entt/entt.hpp>

namespace Engine::Scene
{
    class SceneObject
    {
    public:
        SceneObject();

        ~SceneObject();
    public:
        entt::registry& GetRegistry();
        const entt::registry& GetRegistry() const;

        const std::tuple<entt::entity, Scene::Components::CameraComponent> GetMainCamera() const;

        void AddSystem(UniquePtr<Systems::System> system);

        void Process(const Timer& timer);

    private:
        entt::registry registry;
        std::vector<UniquePtr<Systems::System>> mSystems;
    };
}