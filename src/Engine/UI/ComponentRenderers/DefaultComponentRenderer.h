#pragma once

#include <Types.h>
#include <Scene/Components/ComponentSet.h>
#include <Scene/SceneStorage.h>
#include <entt/entt.hpp>
#include <imgui/imgui.h>

namespace Engine::UI::ComponentRenderers
{
    template <typename TComponent, typename TArg>
    static void RenderComponent(entt::registry& registry, entt::entity entity, TComponent& component, TArg arg)
    {
        auto componentName = Scene::Components::GetComponentName<TComponent>();
        ImGui::LabelText(componentName.data(), "Default component renderer");
    }

    template <typename TComponent>
    static bool HasComponent(const entt::registry& registry, entt::entity entity)
    {
        return registry.all_of<TComponent>(entity);
    }

    template<typename... TComponent, typename TArg>
    static void RenderComponentBlock(Scene::Components::ComponentSet<TComponent...>, entt::registry& registry, entt::entity entity, TArg arg)
    {
        ([&]()
            {
                auto componentName = Scene::Components::GetComponentName<TComponent>();
                if (HasComponent<TComponent>(registry, entity))
                {
                    if (ImGui::CollapsingHeader(componentName.data()))
                    {
                        if (registry.all_of<TComponent>(entity))
                        {
                            TComponent& component = registry.get<TComponent>(entity);
                            RenderComponent<TComponent>(registry, entity, component, arg);
                        }
                        else
                        {
                            TComponent component{};
                            RenderComponent<TComponent>(registry, entity, component, arg);
                        }
                    }
                }
            }(), ...);
    }

    
}