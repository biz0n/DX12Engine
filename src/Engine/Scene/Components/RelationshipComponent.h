#pragma once

#include <Types.h>
#include <entt/entt.hpp>

namespace Engine::Scene::Components
{
    struct RelationshipComponent
    {
        Size childsCount{0};
        Size depth{0};
        entt::entity first{entt::null};
        entt::entity next{entt::null};
        entt::entity parent{entt::null};
    };

    struct Root
    {

    };

    struct Dirty
    {
    };
    
} // namespace Engine::Scene::Components