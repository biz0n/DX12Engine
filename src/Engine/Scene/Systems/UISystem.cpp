#include "UISystem.h"

#include <UIRenderContext.h>
#include <RenderContext.h>

#include <Scene/Components/NameComponent.h>
#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Texture.h>

#include <imgui/imgui.h>
#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    UISystem::UISystem(SharedPtr<RenderContext> renderContext, SharedPtr<UIRenderContext> uiRenderContext)
     : System(), mUIRenderContext(uiRenderContext), mRenderContext(renderContext) {}
    UISystem::~UISystem() = default;

    void UISystem::Process(entt::registry *registry, const Timer &timer)
    {
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        std::function<void(entt::entity, Scene::Components::RelationshipComponent)> showChilds;


        static entt::entity selectedEntity = entt::null;
        showChilds = [&registry, &showChilds](entt::entity e, Scene::Components::RelationshipComponent r)
        {
            const auto &name = registry->get<Scene::Components::NameComponent>(e);

            const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ((selectedEntity == e) ? ImGuiTreeNodeFlags_Selected : 0)
            | (r.First != entt::null ? 0 : (ImGuiTreeNodeFlags_Leaf  | ImGuiTreeNodeFlags_NoTreePushOnOpen));

            if (r.First == entt::null)
            {
                ImGui::TreeNodeEx((void*)(intptr_t)e, node_flags, name.Name.c_str());
                if (ImGui::IsItemClicked())
                {
                    selectedEntity = e;
                }
            }
            else
            {
                bool isOpened = ImGui::TreeNodeEx((void*)(intptr_t)e, node_flags, name.Name.c_str());
                if (ImGui::IsItemClicked())
                {
                    selectedEntity = e;
                }
                if (isOpened)
                {
                    auto child = r.First;
                    while (child != entt::null)
                    {
                        const auto& relationship = registry->get<Scene::Components::RelationshipComponent>(child);
                        showChilds(child, relationship);
                        child = relationship.Next;
                    }    
                    ImGui::TreePop();
                }
            }
        };

        static bool showSceneItems = true;
        if (showSceneItems)
        {
            ImGui::Begin("Scene items", &showSceneItems);
            {
                ImGui::Columns( 2,nullptr,true );
                {
                    ImGui::BeginChild("Scene items area");
                    {
                        const auto& rootEntity = registry->view<Scene::Components::Root>()[0];
                        const auto& relationship = registry->get<Scene::Components::RelationshipComponent>(rootEntity);

                        showChilds(rootEntity, relationship);
                    }
                    ImGui::EndChild();
                }
                ImGui::NextColumn();
                {
                    if (selectedEntity != entt::null)
                    {
                        ImGui::BeginChild("Textures items area");
                        const auto &name = registry->get<Scene::Components::NameComponent>(selectedEntity);

                        ImGui::Text(name.Name.c_str());

                        if (registry->has<Scene::Components::MeshComponent>(selectedEntity))
                        {
                            const auto meshComponent = registry->get<Scene::Components::MeshComponent>(selectedEntity);

                            if (meshComponent.Material->HasBaseColorTexture())
                            {
                                auto texture = meshComponent.Material->GetBaseColorTexture();

                                auto srv = texture->GetShaderResourceView(
                                    mRenderContext->Device(),
                                    mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                                auto textureId = mUIRenderContext->GetTextureId(srv);

                                ImGui::Image(textureId, {256, 256});
                            }

                            if (meshComponent.Material->HasNormalTexture())
                            {
                                auto texture = meshComponent.Material->GetNormalTexture();

                                auto srv = texture->GetShaderResourceView(
                                    mRenderContext->Device(),
                                    mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                                auto textureId = mUIRenderContext->GetTextureId(srv);

                                ImGui::Image(textureId, {256, 256});
                            }

                            if (meshComponent.Material->HasMetallicRoughnessTexture())
                            {
                                auto texture = meshComponent.Material->GetMetallicRoughnessTexture();

                                auto srv = texture->GetShaderResourceView(
                                    mRenderContext->Device(),
                                    mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                                auto textureId = mUIRenderContext->GetTextureId(srv);

                                ImGui::Image(textureId, {256, 256});
                            }
                        }
                        ImGui::EndChild();
                    }
                }

            }
            ImGui::End();
        }
    }

} // namespace Engine::Scene::Systems