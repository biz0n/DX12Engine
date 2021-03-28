#pragma once

#include <Types.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/MeshComponent.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    
    class MeshComponentRenderer : public ComponentRenderer<Engine::Scene::Components::MeshComponent>
    {
        public:
            MeshComponentRenderer()
                : ComponentRenderer("Mesh Component")
             {}
            ~MeshComponentRenderer() override = default;

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::MeshComponent& meshComponent) override
            {
                if (meshComponent.mesh.material->HasBaseColorTexture())
                {
                    auto texture = meshComponent.mesh.material->GetBaseColorTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (meshComponent.mesh.material->HasNormalTexture())
                {
                    auto texture = meshComponent.mesh.material->GetNormalTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (meshComponent.mesh.material->HasMetallicRoughnessTexture())
                {
                    auto texture = meshComponent.mesh.material->GetMetallicRoughnessTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (meshComponent.mesh.material->HasEmissiveTexture())
                {
                    auto texture = meshComponent.mesh.material->GetEmissiveTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (meshComponent.mesh.material->HasAmbientOcclusionTexture())
                {
                    auto texture = meshComponent.mesh.material->GetAmbientOcclusionTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }
            }
    };
}