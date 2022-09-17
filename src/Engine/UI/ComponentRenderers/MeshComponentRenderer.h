#pragma once

#include <Types.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Bin3D/Material.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/MeshResources.h>
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

                if (mSceneStorage->HasTexture(material.BaseColorTextureIndex))
                {
                    auto texture = mSceneStorage->GetTexture(material.BaseColorTextureIndex);

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (mSceneStorage->HasTexture(material.NormalTextureIndex))
                {
                    auto texture = mSceneStorage->GetTexture(material.NormalTextureIndex);

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (mSceneStorage->HasTexture(material.MetallicRoughnessTextureIndex))
                {
                    auto texture = mSceneStorage->GetTexture(material.MetallicRoughnessTextureIndex);

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (mSceneStorage->HasTexture(material.EmissiveTextureIndex))
                {
                    auto texture = mSceneStorage->GetTexture(material.EmissiveTextureIndex);

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }

                if (mSceneStorage->HasTexture(material.AmbientOcclusionTextureIndex))
                {
                    auto texture = mSceneStorage->GetTexture(material.AmbientOcclusionTextureIndex);

                    auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                    ImGui::Image(IMGUI_TEXTURE_ID(srv), {256, 256});
                }
                
            }
    };
}