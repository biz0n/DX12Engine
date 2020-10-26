#pragma once

#include <Timer.h>
#include <Scene/Systems/System.h>
#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/WorldTransformComponent.h>

#include <entt/fwd.hpp>

namespace Engine::Scene::Systems
{
    class WorldTransformSystem : public System
    {
        public:
            WorldTransformSystem();
            ~WorldTransformSystem() override;
        public:
            void Process(entt::registry *registry, const Timer& timer) override;
    };
}