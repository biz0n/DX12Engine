#pragma once

#include <Types.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/MeshComponent.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Scene/SceneStorage.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    
    class MeshComponentRenderer : public ComponentRenderer<Engine::Scene::Components::MeshComponent>
    {
        public:
            MeshComponentRenderer(SharedPtr<Scene::SceneStorage> sceneStorage)
                : ComponentRenderer("Mesh Component"), mSceneStorage{sceneStorage}
             {}
            ~MeshComponentRenderer() override = default;

        private:
            SharedPtr<Scene::SceneStorage> mSceneStorage;

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::MeshComponent& meshComponent) override
            {
                const auto& material = mSceneStorage->GetMaterials()[meshComponent.MaterialIndex];

                if (material.HasBaseColorTexture())
                {
                    auto texture = material.GetBaseColorTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (material.HasNormalTexture())
                {
                    auto texture = material.GetNormalTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (material.HasMetallicRoughnessTexture())
                {
                    auto texture = material.GetMetallicRoughnessTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (material.HasEmissiveTexture())
                {
                    auto texture = material.GetEmissiveTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (material.HasAmbientOcclusionTexture())
                {
                    auto texture = material.GetAmbientOcclusionTexture();

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }
                
            }
    };
}