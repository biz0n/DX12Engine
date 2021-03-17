#pragma once

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/CameraComponent.h>
#include <Scene/Camera.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>
#include <UI/ImGuiEx.h>

#include <DirectXMath.h>

namespace Engine::UI::ComponentRenderers
{
    class CameraComponentRenderer : public ComponentRenderer<Engine::Scene::Components::CameraComponent>
    {
        public:
            CameraComponentRenderer() : ComponentRenderer("Camera Component") {}
            ~CameraComponentRenderer() override = default;

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::CameraComponent& component) override
            {
                auto camera = component.camera;

                const auto farPlane = camera.GetFarPlane();
                const auto nearPlane = camera.GetNearPlane();
                const auto fov = camera.GetFoV();
                const auto type = camera.GetType();

                auto newFoV = dx::XMConvertToDegrees(fov);
                
                auto newNearPlane = nearPlane;
                auto newFarPlane = farPlane;
                auto newType = type;

                bool changed = false;

                std::vector<std::string> keys = { "Perspective", "Orthographic" };
                static int itemCurrent = static_cast<int>(newType);
                if (ImGui::Combo("Type", &itemCurrent, keys))
                {
                    changed = true;
                    camera.SetType(static_cast<Scene::CameraType>(itemCurrent));
                }

                if (camera.GetType() == Scene::CameraType::Perspective && ImGui::SliderFloat("FoV", &newFoV, 1.0f, 180.0f))
                {
                    camera.SetFoV(dx::XMConvertToRadians(newFoV));
                    changed = true;
                }

                if (ImGui::SliderFloat("Near", &newNearPlane, 0.001f, 1000.0f))
                {
                    camera.SetNearPlane(newNearPlane);
                    camera.SetFarPlane(std::max(newNearPlane + 1, newFarPlane));
                    changed = true;
                }

                if (ImGui::SliderFloat("Far", &newFarPlane, newNearPlane + 1, 1000.0f))
                {
                    camera.SetFarPlane(newFarPlane);
                    changed = true;
                }

                if (changed)
                {
                    component.camera = camera;
                    registry.replace<Engine::Scene::Components::CameraComponent>(entity, component);
                }
            }
    };
}