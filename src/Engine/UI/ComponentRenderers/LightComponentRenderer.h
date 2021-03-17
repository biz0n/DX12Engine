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
            ~LightComponentRenderer() override = default;

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::LightComponent& component) override
            {
                auto light = component.light;

                const auto color = light.GetColor();
                const float intensity = light.GetIntensity();
                const float constantAttenuation = light.GetConstantAttenuation();
                const float linearAttenuation = light.GetLinearAttenuation();
                const float quadraticAttenuation = light.GetQuadraticAttenuation();

                float normalizedColor[3] = {color.x, color.y, color.z};
                float newIntensity = intensity;
                
                float newConstantAttenuation = constantAttenuation;
                float newLinearAttenuation = linearAttenuation;
                float newQuadraticAttenuation = quadraticAttenuation;

                const std::string types[3] = {"Directiona", "Point", "Spot"};
                ImGui::LabelText("Type", types[(uint32)light.GetLightType()].c_str());

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

                if (ImGui::SliderFloat("ConstantAttenuation", &newConstantAttenuation, 0.0f, 10.0f))
                {
                    light.SetConstantAttenuation(newConstantAttenuation);
                    changed = true;
                }

                if (ImGui::SliderFloat("LinearAttenuation", &newLinearAttenuation, 0.0f, 10.0f))
                {
                    light.SetLinearAttenuation(newLinearAttenuation);
                    changed = true;
                }

                if (ImGui::SliderFloat("QuadraticAttenuation", &newQuadraticAttenuation, 0.0f, 10.0f))
                {
                    light.SetQuadraticAttenuation(newQuadraticAttenuation);
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