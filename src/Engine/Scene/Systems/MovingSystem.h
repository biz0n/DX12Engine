#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/Systems/System.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Keyboard.h>

#include <entt/fwd.hpp>

namespace Engine::Scene::Systems
{
    class MovingSystem : public System
    {
        public:
            MovingSystem(SharedPtr<Keyboard> keyboard);
            ~MovingSystem() override;
        public:
            void Init(entt::registry *registry) override;
            void Process(entt::registry *registry, const Timer& timer) override;
        private:
            SharedPtr<Keyboard> mKeyboard;

    };
}