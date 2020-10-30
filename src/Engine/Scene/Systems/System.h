#pragma once

#include <Timer.h>

#include <entt/fwd.hpp>

namespace Engine::Scene::Systems
{
    class System
    {
        public:
            System(){}
            virtual ~System() = 0;
        public:
            virtual void Init(entt::registry *registry){}
            virtual void Process(entt::registry *registry, const Timer& timer){}
            
    };

    inline System::~System() = default;
}