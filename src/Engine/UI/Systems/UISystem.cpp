#include "UISystem.h"

#include <Render/UIRenderContext.h>
#include <Render/RenderContext.h>
#include <HAL/SwapChain.h>
#include <Memory/Texture.h>

#include <Scene/SceneRegistry.h>
#include <Scene/SceneStorage.h>
#include <Scene/Components/ComponentSet.h>


#include <UI/ComponentRenderers/StateComponentsRenderer.h>
#include <UI/ComponentRenderers/WorldTransformComponentRenderer.h>
#include <UI/ComponentRenderers/MeshComponentRenderer.h>
#include <UI/ComponentRenderers/LightComponentRenderer.h>
#include <UI/ComponentRenderers/CameraComponentRenderer.h>
#include <UI/ComponentRenderers/DefaultComponentRenderer.h>

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <entt/entt.hpp>

namespace Engine::UI::Systems
{
    UISystem::UISystem(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Scene::SceneStorage> sceneStorage, SharedPtr<UIContext> uiContext)
        : System(), mRenderContext{ renderContext }, mSceneStorage{ sceneStorage }, mUiContext{ uiContext }
    {
    }

    UISystem::~UISystem() = default;

    void UISystem::Process(Scene::SceneRegistry* scene, const Timer &timer)
    {
        auto& registry = scene->GetRegistry();
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        std::function<void(entt::entity, Scene::Components::RelationshipComponent)> showChilds;

        if (!registry.valid(mUiContext->SelectedEntity))
        {
            mUiContext->SelectedEntity = entt::null;
        }

        showChilds = [&registry, &showChilds, this](entt::entity e, Scene::Components::RelationshipComponent r) {
            const auto &name = registry.get<Scene::Components::NameComponent>(e);

            const auto node_flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ((mUiContext->SelectedEntity == e) ? ImGuiTreeNodeFlags_Selected : 0) |
                (r.first != entt::null ? 0 : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen));

            
            if (r.first == entt::null)
            {
                ImGui::TreeNodeEx((void *)(intptr_t)e, node_flags, "%i: %s(%i)", e, name.Name.c_str(), r.depth);
                if (ImGui::IsItemClicked())
                {
                    mUiContext->SelectedEntity = e;
                }
            }
            else
            {
                bool isOpened = ImGui::TreeNodeEx((void *)(intptr_t)e, node_flags, "%i: %s(%i) [%i]", e, name.Name.c_str(), r.depth, r.childsCount);
                if (ImGui::IsItemClicked())
                {
                    mUiContext->SelectedEntity = e;
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
                    if (mUiContext->SelectedEntity != entt::null)
                    {
                        ComponentRenderers::RenderComponentBlock(Scene::Components::AllComponents{}, registry, mUiContext->SelectedEntity, mSceneStorage);
                    }
                    ImGui::EndChild();
                }
            }
            ImGui::End();
        }

        ImGui::Begin("Cameras");
        {
            int currentCameraIndex;
            entt::entity currentCameraEntity;
            std::vector<std::string> cameraNames;
            std::vector<entt::entity> cameraEntities;
            for (auto&& [entity, camera, name] : registry.view<Scene::Components::CameraComponent, Scene::Components::NameComponent>().each())
            {
                if (registry.all_of<Scene::Components::MainCameraComponent>(entity))
                {
                    currentCameraIndex = cameraNames.size();
                    currentCameraEntity = entity;
                }
                cameraNames.push_back(name.Name);
                cameraEntities.push_back(entity);

                dx::XMFLOAT3 corners[8];
                camera.frustum.GetCorners(corners);
            }

            if (ImGui::Combo("Select Camera", &currentCameraIndex, cameraNames))
            {
                registry.remove<Scene::Components::MainCameraComponent>(currentCameraEntity);
                currentCameraEntity = cameraEntities[currentCameraIndex];
                registry.emplace<Scene::Components::MainCameraComponent>(currentCameraEntity);
            }
        }
        ImGui::End();

        

        if (mUiContext->SelectedEntity != entt::null && registry.all_of<Scene::Components::WorldTransformComponent>(mUiContext->SelectedEntity))
        {
            auto [cameraEntity, camera] = scene->GetMainCamera();
            auto worldTransformComponent = registry.get<Scene::Components::WorldTransformComponent>(mUiContext->SelectedEntity);
            if (mUiContext->SelectedEntity == cameraEntity)
            {
                return;
            }
            

            dx::XMFLOAT4X4 view;
            dx::XMFLOAT4X4 proj; 
            dx::XMFLOAT4X4 matrix; 
            dx::XMStoreFloat4x4(&view, camera.view);
            dx::XMStoreFloat4x4(&proj, camera.projection);
            dx::XMStoreFloat4x4(&matrix, worldTransformComponent.transform);

           // ImGuizmo::Manipulate(*view.m, *proj.m, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, *matrix.m);

            // ImGuizmo::DrawCubes(*view.m, *proj.m, *matrix.m, 1);

            
            
        }
    }

} // namespace Engine::Scene::Systems