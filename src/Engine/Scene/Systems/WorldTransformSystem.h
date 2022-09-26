#pragma once

#include <Timer.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    class WorldTransformSystem : public System
    {
        public:
            WorldTransformSystem();
            ~WorldTransformSystem() override;
        public:
            void Init(SceneRegistry* scene) override;
            void Process(SceneRegistry* scene, const Timer& timer) override;
        private:
            void InitWithDirty(entt::registry& r, entt::entity entity);
            void MarkAsDirty(entt::registry& r, entt::entity entity);

    };
}