#pragma once

#include <Types.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Camera.h>
#include <Scene/Components/CameraComponent.h>
#include <MathUtils.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

#include <DirectXMath.h>

namespace Engine::UI::ComponentRenderers
{
    
    class WorldTransformComponentRenderer : public ComponentRenderer<Engine::Scene::Components::WorldTransformComponent>
    {
        public:
            WorldTransformComponentRenderer() : ComponentRenderer("WorldTransform Component") {}
            ~WorldTransformComponentRenderer() override = default;

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::WorldTransformComponent& component) override
            {
                auto matrix = component.transform;
                DirectX::XMVECTOR scaleVector;
                DirectX::XMVECTOR rotationQuaternion;
                DirectX::XMVECTOR positionVector;
                DirectX::XMMatrixDecompose(&scaleVector, &rotationQuaternion, &positionVector, matrix);

                DirectX::XMFLOAT3 scale{};
                DirectX::XMFLOAT3 position{};
                
                DirectX::XMStoreFloat3(&scale, scaleVector);
                DirectX::XMStoreFloat3(&position, positionVector);

                auto localTransform = registry.get<Scene::Components::LocalTransformComponent>(entity);

                auto cameraEntity = registry.view<Scene::Components::CameraComponent, Scene::Components::MainCameraComponent>().front();
                auto camera = registry.get<Scene::Components::CameraComponent>(cameraEntity);

                dx::XMMATRIX parentWorldTransform = dx::XMMatrixIdentity();
                auto parentEntity = registry.get<Scene::Components::RelationshipComponent>(entity).parent;
                if (parentEntity != entt::null)
                {
                    parentWorldTransform = registry.get<Scene::Components::WorldTransformComponent>(parentEntity).transform;
                }

                dx::XMFLOAT4X4 localMatrix{};
                dx::XMStoreFloat4x4(&localMatrix, localTransform.transform);

                auto result = EditTransform(camera, localMatrix, parentWorldTransform);

                registry.emplace_or_replace<Scene::Components::LocalTransformComponent>(entity, dx::XMLoadFloat4x4(&result));

            }

            dx::XMFLOAT4X4 EditTransform(const Scene::Components::CameraComponent& camera, dx::XMFLOAT4X4& matrix, const dx::XMMATRIX& parentWorldTransform)
            {
                static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
                const static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

                if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                {
                    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                }

                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
                {
                    mCurrentGizmoOperation = ImGuizmo::ROTATE;
                }

                ImGui::SameLine();
                if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
                {
                    mCurrentGizmoOperation = ImGuizmo::SCALE;
                }

                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(static_cast<float*>((void*)matrix.m), matrixTranslation, matrixRotation, matrixScale);
                ImGui::InputFloat3("Tr", matrixTranslation);
                ImGui::InputFloat3("Rt", matrixRotation);
                ImGui::InputFloat3("Sc", matrixScale);
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, static_cast<float*>((void*)&matrix));

                static bool useSnap(false);

                ImGui::Checkbox("Use Snap", &useSnap);
                ImGui::SameLine();
                static dx::XMFLOAT3 snap;
                switch (mCurrentGizmoOperation)
                {
                case ImGuizmo::TRANSLATE:
                    snap = {1, 1, 1};
                    ImGui::InputFloat3("Snap", &snap.x);
                    break;
                case ImGuizmo::ROTATE:
                    snap.x = 5;
                    ImGui::InputFloat("Angle Snap", &snap.x);
                    break;
                case ImGuizmo::SCALE:
                    snap.x = 0.001f;
                    ImGui::InputFloat("Scale Snap", &snap.x);
                    break;
                }
                ImGuiIO& io = ImGui::GetIO();
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                dx::XMFLOAT4X4 view{};
                dx::XMFLOAT4X4 projection{};

                using namespace dx;
                dx::XMStoreFloat4x4(&view, camera.view * parentWorldTransform);
                dx::XMStoreFloat4x4(&projection, camera.projection);

                ImGuizmo::Manipulate(
                    static_cast<float*>((void*)&view),
                    static_cast<float*>((void*)&projection),
                    mCurrentGizmoOperation,
                    mCurrentGizmoMode,
                    static_cast<float*>((void*)&matrix),
                    nullptr,
                    useSnap ? &snap.x : nullptr);

                return matrix;
            }
    };
}