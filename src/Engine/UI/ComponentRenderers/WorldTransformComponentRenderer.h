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
#include <imgui/ImGuizmo.h>

#include <DirectXMath.h>

namespace Engine::UI::ComponentRenderers
{
    
    class WorldTransformComponentRenderer : public ComponentRenderer<Engine::Scene::Components::WorldTransformComponent>
    {
        public:
            WorldTransformComponentRenderer() : ComponentRenderer("WorldTransform Component") {}
            ~WorldTransformComponentRenderer() {}

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::WorldTransformComponent& component) override
            {
                auto matrix = component.transform;
                DirectX::XMVECTOR scaleVector;
                DirectX::XMVECTOR rotationQuaternion;
                DirectX::XMVECTOR positionVector;
                DirectX::XMMatrixDecompose(&scaleVector, &rotationQuaternion, &positionVector, matrix);

                DirectX::XMFLOAT3 scale;
                DirectX::XMFLOAT3 position;
                
                DirectX::XMStoreFloat3(&scale, scaleVector);
                DirectX::XMStoreFloat3(&position, positionVector);

                float pitch, yaw, roll;
                Math::ExtractPitchYawRollFromXMMatrix(&pitch, &yaw, &roll, &matrix);
                //float pitch;
                //DirectX::XMQuaternionToAxisAngle(&rotationQuaternion, &pitch, )
                
                //DirectX::XMStoreFloat3
                float rt[3] = { DirectX::XMConvertToDegrees(pitch), DirectX::XMConvertToDegrees(yaw), DirectX::XMConvertToDegrees(roll)};
              //  ImGui::InputFloat3("Tr", (float*)&position, "%.2f", ImGuiInputTextFlags_ReadOnly);
              //  ImGui::InputFloat3("Rt", rt, "%.2f", ImGuiInputTextFlags_ReadOnly);
              //  ImGui::InputFloat3("Sc", (float*)&scale, "%.2f", ImGuiInputTextFlags_ReadOnly);


                auto localTransform = registry.get<Scene::Components::LocalTransformComponent>(entity);

                auto cameraEntity = registry.view<Scene::Components::CameraComponent, Scene::Components::MainCameraComponent>().front();
                auto camera = registry.get<Scene::Components::CameraComponent>(cameraEntity);
                

                dx::XMFLOAT4X4 m;
                dx::XMStoreFloat4x4(&m, matrix);   

                auto delta = EditTransform(camera, m);

                using namespace dx;

                if (!dx::XMMatrixIsIdentity(dx::XMLoadFloat4x4(&delta)))
                {
                    registry.emplace_or_replace<Scene::Components::LocalTransformComponent>(entity, localTransform.transform * dx::XMLoadFloat4x4(&delta));
                }
            }

            dx::XMFLOAT4X4 EditTransform(const Scene::Components::CameraComponent& camera, dx::XMFLOAT4X4& matrix)
            {
                static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
                static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
                /*if (ImGui::IsKeyPressed(90))
                    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                if (ImGui::IsKeyPressed(69))
                    mCurrentGizmoOperation = ImGuizmo::ROTATE;
                if (ImGui::IsKeyPressed(82)) // r Key
                    mCurrentGizmoOperation = ImGuizmo::SCALE;*/
                if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
                    mCurrentGizmoOperation = ImGuizmo::ROTATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
                    mCurrentGizmoOperation = ImGuizmo::SCALE;
                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(static_cast<float*>((void*)matrix.m), matrixTranslation, matrixRotation, matrixScale);
                ImGui::InputFloat3("Tr", matrixTranslation);
                ImGui::InputFloat3("Rt", matrixRotation);
                ImGui::InputFloat3("Sc", matrixScale);
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, static_cast<float*>((void*)&matrix));

                if (mCurrentGizmoOperation != ImGuizmo::SCALE)
                {
                    if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                        mCurrentGizmoMode = ImGuizmo::LOCAL;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                        mCurrentGizmoMode = ImGuizmo::WORLD;
                }
                static bool useSnap(false);
            //    if (ImGui::IsKeyPressed(83))
            //        useSnap = !useSnap;
                ImGui::Checkbox("", &useSnap);
                ImGui::SameLine();
                dx::XMFLOAT3 snap;
                switch (mCurrentGizmoOperation)
                {
                case ImGuizmo::TRANSLATE:
                    snap = {1, 1, 1};// config.mSnapTranslation;
                    ImGui::InputFloat3("Snap", &snap.x);
                    break;
                case ImGuizmo::ROTATE:
                    snap.x = 5;//config.mSnapRotation;
                    ImGui::InputFloat("Angle Snap", &snap.x);
                    break;
                case ImGuizmo::SCALE:
                    snap.x = 0.1f;//config.mSnapScale;
                    ImGui::InputFloat("Scale Snap", &snap.x);
                    break;
                }
                ImGuiIO& io = ImGui::GetIO();
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                dx::XMFLOAT4X4 view;
                dx::XMFLOAT4X4 projection;

                dx::XMStoreFloat4x4(&view, camera.view);
                dx::XMStoreFloat4x4(&projection, camera.projection);

                dx::XMFLOAT4X4 delta = {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                };

                ImGuizmo::Manipulate(
                    static_cast<float*>((void*)&view),
                    static_cast<float*>((void*)&projection),
                    mCurrentGizmoOperation,
                    mCurrentGizmoMode,
                    static_cast<float*>((void*)&matrix),
                    static_cast<float*>((void*)&delta),
                    useSnap ? &snap.x : NULL);

                return delta;
            }
    };
}