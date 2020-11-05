#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/Systems/System.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/SceneForwards.h>
#include <IO/Keyboard.h>

#include <entt/fwd.hpp>

namespace Engine::Scene::Systems
{
    class MovingSystem : public System
    {
        public:
            MovingSystem(SharedPtr<Keyboard> keyboard);
            ~MovingSystem() override;
        public:
            void Init(SceneObject *scene) override;
            void Process(SceneObject *scene, const Timer& timer) override;
        private:
            SharedPtr<Keyboard> mKeyboard;

    };
}