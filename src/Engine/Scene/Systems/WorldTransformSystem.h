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
            void Init(entt::registry *registry) override;
            void Process(entt::registry *registry, const Timer& timer) override;
        private:
            void InitWithDirty(entt::registry& r, entt::entity entity);
            void MarkAsDirty(entt::registry& r, entt::entity entity);

    };
}