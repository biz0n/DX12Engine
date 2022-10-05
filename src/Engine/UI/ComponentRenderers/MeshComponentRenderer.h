#pragma once

#include <Types.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>

#include <UI/ComponentRenderers/DefaultComponentRenderer.h>

#include <Bin3D/Material.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/MeshResources.h>


#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    
    template <>
    static void RenderComponent<Scene::Components::MeshComponent>(
        entt::registry& registry, 
        entt::entity entity, 
        Scene::Components::MeshComponent& component, 
        SharedPtr<Scene::SceneStorage> sceneStorage)
    {
        const auto& material = sceneStorage->GetMaterials()[component.MaterialIndex];

        std::function drawTexture = [sceneStorage](uint16_t textureIndex)
        {
            if (sceneStorage->HasTexture(textureIndex))
            {
                auto texture = sceneStorage->GetTexture(textureIndex);
                
                ImGui::TextUnformatted(texture->GetName().c_str());
                const auto& description = texture->GetDescription();
                const float aspect = (float)description.Height / description.Width;

                auto srv = texture->GetSRDescriptor().GetGPUDescriptor();

                ImGui::Image(IMGUI_TEXTURE_ID(srv), { 256, 256 * aspect });
            }
        };

        drawTexture(material.BaseColorTextureIndex);
        drawTexture(material.NormalTextureIndex);
        drawTexture(material.MetallicRoughnessTextureIndex);
        drawTexture(material.EmissiveTextureIndex);
        drawTexture(material.AmbientOcclusionTextureIndex);
    }
}