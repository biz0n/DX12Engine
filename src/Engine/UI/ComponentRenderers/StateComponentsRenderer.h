#pragma once

#include <Types.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Scene/Components/MeshComponent.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    class StateComponentsRenderer : public ComponentRendererBase
    {
        public:
            StateComponentsRenderer()
                : ComponentRendererBase("State")
             {}
            ~StateComponentsRenderer() override = default;

        public:
            bool HasComponent(entt::registry& registry, entt::entity entity) const override
            {
                return registry.all_of<Scene::Components::MeshComponent>(entity);
            }

            void RenderComponent(entt::registry& registry, entt::entity entity) override
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
    };
}