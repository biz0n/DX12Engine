#pragma once

#include <Timer.h>
#include <Scene/SceneForwards.h>

namespace Engine::Scene::Systems
{
    class System
    {
        public:
            System(){}
            virtual ~System() = 0;
        public:
            virtual void Init(SceneRegistry*scene){}
            virtual void Process(SceneRegistry*scene, const Timer& timer){}
            
    };

    inline System::~System() = default;
}