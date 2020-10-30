#pragma once

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/WorldTransformComponent.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>

#include <DirectXMath.h>

namespace Engine::UI::ComponentRenderers
{
    
    class WorldTransformComponentRenderer : public ComponentRenderer<Engine::Scene::Components::WorldTransformComponent>
    {
        public:
            WorldTransformComponentRenderer() {}
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
                ExtractPitchYawRollFromXMMatrix(&pitch, &yaw, &roll, &matrix);
                //float pitch;
                //DirectX::XMQuaternionToAxisAngle(&rotationQuaternion, &pitch, )
                
                //DirectX::XMStoreFloat3
                float rt[3] = { DirectX::XMConvertToDegrees(pitch), DirectX::XMConvertToDegrees(yaw), DirectX::XMConvertToDegrees(roll)};
                ImGui::InputFloat3("Tr", (float*)&position, 2, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat3("Rt", rt, 2, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat3("Sc", (float*)&scale, 2, ImGuiInputTextFlags_ReadOnly);
            }

            void ExtractPitchYawRollFromXMMatrix(float* flt_p_PitchOut, float* flt_p_YawOut, float* flt_p_RollOut, const DirectX::XMMATRIX* XMMatrix_p_Rotation)
            {
                DirectX::XMFLOAT4X4 XMFLOAT4X4_Values;
                DirectX::XMStoreFloat4x4(&XMFLOAT4X4_Values, DirectX::XMMatrixTranspose(*XMMatrix_p_Rotation));
                *flt_p_PitchOut = (float)asin(-XMFLOAT4X4_Values._23);
                *flt_p_YawOut = (float)atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33);
                *flt_p_RollOut = (float)atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22);
            }
    };
}