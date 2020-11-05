#pragma once

#include <Timer.h>
#include <Scene/SceneForwards.h>

#include <entt/fwd.hpp>

namespace Engine::Scene::Systems
{
    class System
    {
        public:
            System(){}
            virtual ~System() = 0;
        public:
            virtual void Init(SceneObject *scene){}
            virtual void Process(SceneObject *scene, const Timer& timer){}
            
    };

    inline System::~System() = default;
}