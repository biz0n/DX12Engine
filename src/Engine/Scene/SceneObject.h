#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>

#include <Scene/Systems/System.h>
#include <vector>
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

        void AddSystem(UniquePtr<Systems::System> system);

        void Process(const Timer& timer);

    private:
        entt::registry registry;
        std::vector<UniquePtr<Systems::System>> mSystems;
    };
}