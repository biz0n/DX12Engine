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
            registry = MakeUnique<entt::registry>();

            
        }

    public:
        UniquePtr<entt::registry> registry;

    public:
        void AddSystem(UniquePtr<Systems::System> system)
        {
            system->Init(registry.get());
            mSystems.push_back(std::move(system));
        }

        void Process(const Timer& timer)
        {
            for (int i = 0; i < mSystems.size(); ++i)
            {
                mSystems[i]->Process(registry.get(), timer);
            }
        }

    private:
        std::vector<UniquePtr<Systems::System>> mSystems;
    };
}