#pragma once

#include <entt/entt.hpp>

namespace Engine::UI
{
    class UIContext
    {
    public:
        entt::entity SelectedEntity = entt::null;
    };
}