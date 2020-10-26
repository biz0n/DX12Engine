#include "UISystem.h"

#include <UIRenderContext.h>

#include <Scene/Components/NameComponent.h>
#include <Scene/Components/RelationshipComponent.h>

#include <imgui/imgui.h>
#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    UISystem::UISystem(SharedPtr<UIRenderContext> renderContext)
     : System(), mRenderContext(renderContext) {}
    UISystem::~UISystem() = default;

    void UISystem::Process(entt::registry *registry, const Timer &timer)
    {
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        std::function<void(entt::entity, Scene::Components::RelationshipComponent)> showChilds;

        showChilds = [&registry, &showChilds](entt::entity e, Scene::Components::RelationshipComponent r)
        {
            auto name = registry->get<Scene::Components::NameComponent>(e);

            if (r.First == entt::null)
            {
                ImGui::Text(name.Name.c_str());
            }
            else
            {
                if (ImGui::TreeNode(name.Name.c_str()))
                {
                    auto child = r.First;
                    while (child != entt::null)
                    {
                        auto relationship = registry->get<Scene::Components::RelationshipComponent>(child);
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
                auto rootEntity = registry->view<Scene::Components::Root>()[0];
                auto relationship = registry->get<Scene::Components::RelationshipComponent>(rootEntity);

                showChilds(rootEntity, relationship);
            }
            ImGui::End();
        }
    }

} // namespace Engine::Scene::Systems