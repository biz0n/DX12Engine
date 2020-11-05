#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>

#include <Scene/Systems/System.h>
#include <vector>
#include <entt/fwd.hpp>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>

namespace Engine::Scene
{
    class SceneObject
    {
    public:
        SceneObject()
        {
        }
    public:
        entt::registry& GetRegistry() { return registry; }

        void AddSystem(UniquePtr<Systems::System> system)
        {
            system->Init(this);
            mSystems.push_back(std::move(system));
        }

        void Process(const Timer& timer)
        {
            for (int i = 0; i < mSystems.size(); ++i)
            {
                mSystems[i]->Process(this, timer);
            }
        }

    private:
        entt::registry registry;
        std::vector<UniquePtr<Systems::System>> mSystems;
    };
}