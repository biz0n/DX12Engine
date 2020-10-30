#pragma once

#include <Types.h>
#include <entt/entt.hpp>

namespace Engine::Scene::Components
{
    struct RelationshipComponent
    {
        Size ChildsCount{0};
        entt::entity First{entt::null};
       // entt::entity Prev{entt::null};
        entt::entity Next{entt::null};
        entt::entity Parent{entt::null};
    };

    struct Root
    {

    };

    struct Dirty
    {
    };
    
} // namespace Engine::Scene::Components