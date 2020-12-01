#include "UISystem.h"

#include <Render/UIRenderContext.h>
#include <Render/RenderContext.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Texture.h>

#include <UI/ComponentRenderers./StateComponentsRenderer.h>
#include <UI/ComponentRenderers/WorldTransformComponentRenderer.h>
#include <UI/ComponentRenderers/MeshComponentRenderer.h>
#include <UI/ComponentRenderers/LightComponentRenderer.h>

#include <imgui/imgui.h>
#include <entt/entt.hpp>

namespace Engine::UI::Systems
{
    UISystem::UISystem(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Render::UIRenderContext> uiRenderContext)
        : System(), mUIRenderContext(uiRenderContext), mRenderContext(renderContext)
    {
        mComponentRenderers.push_back(MakeUnique<UI::ComponentRenderers::StateComponentsRenderer>());
        mComponentRenderers.push_back(MakeUnique<UI::ComponentRenderers::WorldTransformComponentRenderer>());
        mComponentRenderers.push_back(MakeUnique<UI::ComponentRenderers::MeshComponentRenderer>(renderContext, uiRenderContext));
        mComponentRenderers.push_back(MakeUnique<UI::ComponentRenderers::LightComponentRenderer>());
    }

    UISystem::~UISystem() = default;

    void UISystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        auto& registry = scene->GetRegistry();
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        std::function<void(entt::entity, Scene::Components::RelationshipComponent)> showChilds;

        static entt::entity selectedEntity = entt::null;
        showChilds = [&registry, &showChilds](entt::entity e, Scene::Components::RelationshipComponent r) {
            const auto &name = registry.get<Scene::Components::NameComponent>(e);

            const auto node_flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ((selectedEntity == e) ? ImGuiTreeNodeFlags_Selected : 0) |
                (r.first != entt::null ? 0 : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen));

            
            if (r.first == entt::null)
            {
                ImGui::TreeNodeEx((void *)(intptr_t)e, node_flags, "%i: %s(%i)", e, name.Name.c_str(), r.depth);
                if (ImGui::IsItemClicked())
                {
                    selectedEntity = e;
                }
            }
            else
            {
                bool isOpened = ImGui::TreeNodeEx((void *)(intptr_t)e, node_flags, "%i: %s(%i) [%i]", e, name.Name.c_str(), r.depth, r.childsCount);
                if (ImGui::IsItemClicked())
                {
                    selectedEntity = e;
                }
                if (isOpened)
                {
                    auto child = r.first;
                    while (child != entt::null)
                    {
                        const auto &relationship = registry.get<Scene::Components::RelationshipComponent>(child);
                        showChilds(child, relationship);
                        child = relationship.next;
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
                ImGui::Columns(2, nullptr, true);
                {
                    ImGui::BeginChild("Scene items area");
                    {
                        const auto& roots = registry.view<Scene::Components::Root>();
                        for (auto rootEntity : roots)
                        {
                            const auto &relationship = registry.get<Scene::Components::RelationshipComponent>(rootEntity);

                            showChilds(rootEntity, relationship);
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::NextColumn();
                {
                    ImGui::BeginChild("Components area");
                    if (selectedEntity != entt::null)
                    {
                        for (auto &r : mComponentRenderers)
                        {
                            if (r->HasComponent(registry, selectedEntity))
                            {
                                if (ImGui::CollapsingHeader(r->Name().c_str()))
                                {
                                    r->RenderComponent(registry, selectedEntity);
                                }
                            }
                        }
                    }
                    ImGui::EndChild();
                }
            }
            ImGui::End();
        }
    }

} // namespace Engine::Scene::Systems