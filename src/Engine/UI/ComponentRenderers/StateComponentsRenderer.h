#pragma once

#include <Types.h>

#include <UI/ComponentRenderers/DefaultComponentRenderer.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Scene/Components/MeshComponent.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    template <>
    static bool HasComponent<Scene::Components::IsDisabledComponent>(const entt::registry& registry, entt::entity entity)
    {
        return registry.all_of<Scene::Components::MeshComponent>(entity);
    }

    template <>
    static void RenderComponent<Scene::Components::IsDisabledComponent>(
        entt::registry& registry, 
        entt::entity entity, 
        Scene::Components::IsDisabledComponent& component, 
        SharedPtr<Scene::SceneStorage>)
    {
        bool isActive = !registry.all_of<Scene::Components::IsDisabledComponent>(entity);
        if (ImGui::Checkbox("Is Active", &isActive))
        {
            if (isActive)
            {
                registry.remove<Scene::Components::IsDisabledComponent>(entity);
            }
            else
            {
                registry.emplace_or_replace<Scene::Components::IsDisabledComponent>(entity);
            }
        }
    }
}