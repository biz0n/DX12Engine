#pragma once

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/LightComponent.h>
#include <Scene/PunctualLight.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>

#include <DirectXMath.h>

namespace Engine::UI::ComponentRenderers
{
    
    class LightComponentRenderer : public ComponentRenderer<Engine::Scene::Components::LightComponent>
    {
        public:
            LightComponentRenderer() : ComponentRenderer("Light Component") {}
            ~LightComponentRenderer() {}

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::LightComponent& component) override
            {
                auto light = component.light;

                const auto color = light.GetColor();
                const float intensity = light.GetIntensity();

                float normalizedColor[3] = {color.x, color.y, color.z};
                float newIntensity = intensity;

                bool changed = false;
                if (ImGui::SliderFloat("Intensity", &newIntensity, 0.1f, 100.0f))
                {
                    light.SetIntensity(newIntensity);
                    changed = true;
                }

                if (ImGui::ColorEdit3("Color", normalizedColor))
                {
                    dx::XMFLOAT3 newColor = { (normalizedColor[0]), (normalizedColor[1]), (normalizedColor[2])};
                    light.SetColor(newColor);
                    changed = true;
                }

                if (changed)
                {
                    component.light = light;
                    registry.replace<Engine::Scene::Components::LightComponent>(entity, component);
                }
            }
    };
}