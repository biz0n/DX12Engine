#include "LightCameraSystem.h"

#include <EngineConfig.h>

#include <Render/RenderContext.h>
#include <HAL/SwapChain.h>

#include <Scene/SceneObject.h>
#include <Scene/PunctualLight.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/WorldTransformComponent.h>

#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/IsDisabledComponent.h>

#include <entt/entt.hpp>
#include <cmath>
#include <DirectXCollision.h>
#include <vector>

namespace Engine::Scene::Systems
{
    LightCameraSystem::LightCameraSystem(SharedPtr<Render::RenderContext> renderContext) : mRenderContext(renderContext)
    {
    }

    LightCameraSystem::~LightCameraSystem() = default;

    void LightCameraSystem::Process(SceneObject *scene, const Timer &timer)
    {
        auto& registry = scene->GetRegistry();

        const auto& view = registry.view<Components::CameraComponent, Components::WorldTransformComponent, Components::LightComponent>();

        const float32 width = static_cast<float32>(mRenderContext->GetSwapChain()->GetWidth());
        const float32 height = static_cast<float32>(mRenderContext->GetSwapChain()->GetHeight());
        
        static const dx::XMVECTOR up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        static const dx::XMVECTOR forward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        const auto [ce, mainCamera] = scene->GetMainCamera();

        for (auto &&[entity, cameraComponent, transformComponent, lightComponent] : view.each())
        {
            auto& camera = cameraComponent.camera;
            const auto& world = transformComponent.transform;
            const auto& light = lightComponent.light;

            using namespace dx;

            dx::XMVECTOR unused;
            dx::XMVECTOR rt;
            dx::XMVECTOR tr;
            dx::XMMatrixDecompose(&unused, &rt, &tr, world);

            dx::XMMATRIX viewMatrix;
            dx::XMMATRIX projectionMatrix;
            CameraType cameraType;

            dx::XMVECTOR direction = dx::XMVector4Transform(forward, world);

            switch(light.GetLightType())
            {
                case LightType::SpotLight:
                {
                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    camera.SetType(CameraType::Perspective);
                    projectionMatrix = camera.GetProjectionMatrix(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);
                    camera.SetFoV(light.GetOuterConeAngle());
                    cameraType = CameraType::Perspective;
                }
                break;
                case LightType::PointLight: // not supported for now, but light will show only forward camera
                    viewMatrix = dx::XMMatrixLookToLH(tr, forward, up);
                    camera.SetType(CameraType::Perspective);
                    projectionMatrix = camera.GetProjectionMatrix(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);
                    camera.SetFoV(dx::XMConvertToRadians(90));
                    cameraType = CameraType::Perspective;
                break;
                case LightType::DirectionalLight:
                {
                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    dx::XMVECTOR D;
                    const auto inverseView = dx::XMMatrixInverse(&D, viewMatrix);
                    dx::XMMatrixDecompose(&unused, &rt, &unused, inverseView);
                    camera.SetType(CameraType::Orthographic);

                    const auto &boundingBoxes = registry.view<Scene::Components::AABBComponent>(entt::exclude<Scene::Components::IsDisabledComponent>);

                    std::vector<dx::XMFLOAT3> points;
                    dx::XMFLOAT3 corners[dx::BoundingBox::CORNER_COUNT];
                    points.reserve(boundingBoxes.size_hint() * dx::BoundingBox::CORNER_COUNT);
                    for (auto&& [entity, aabb] : boundingBoxes.each())
                    {
                        aabb.boundingBox.GetCorners(corners);

                        for (size_t i = 0; i < dx::BoundingBox::CORNER_COUNT; ++i)
                        {
                            points.push_back(corners[i]);
                        }
                    }

                    dx::BoundingBox box;
                    dx::BoundingBox::CreateFromPoints(box, points.size(), points.data(), sizeof(dx::XMFLOAT3));

                    box.GetCorners(corners);

                    for (size_t i = 0; i < dx::BoundingBox::CORNER_COUNT; ++i)
                    {
                        auto corner = dx::XMLoadFloat3(&corners[i]);
                        dx::XMStoreFloat3(
                            &corners[i], 
                            dx::XMVector3Transform(corner, viewMatrix));
                    }
                    dx::BoundingBox::CreateFromPoints(box, std::size(corners), corners, sizeof(dx::XMFLOAT3));

                    const auto boxCenter = dx::XMVector3Transform(dx::XMLoadFloat3(&box.Center), inverseView);
                    const auto boxExtentsLength = dx::XMVectorGetX(dx::XMVector3Length(dx::XMLoadFloat3(&box.Extents)));
                    const auto boxHalfDepth = box.Extents.z;
                    const auto boxWidth = box.Extents.x;
                    const auto boxHeight = box.Extents.y;

                    tr = boxCenter - dx::XMVector3Normalize(direction) * boxExtentsLength;

                    const float32 nearPlane = dx::XMVectorGetZ(dx::XMVector3Length(tr - boxCenter)) - boxHalfDepth;
                    const float32 farPlane = nearPlane + boxHalfDepth + boxHalfDepth;

                    camera.SetNearPlane(nearPlane);
                    camera.SetFarPlane(farPlane);

                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    const auto maxDimension = 2 * std::max(boxWidth, boxHeight);
                    projectionMatrix = camera.GetProjectionMatrix(maxDimension, maxDimension);
                }
                break;
            }

            dx::XMMATRIX vpMatrix = dx::XMMatrixMultiply(viewMatrix, projectionMatrix);
            vpMatrix = dx::XMMatrixTranspose(vpMatrix);

            cameraComponent.eyePosition = tr;
            cameraComponent.view = viewMatrix;
            cameraComponent.projection = projectionMatrix;
            cameraComponent.viewProjection = vpMatrix;

            dx::BoundingFrustum frustum {projectionMatrix};

            dx::XMStoreFloat3(&frustum.Origin, tr);
            dx::XMStoreFloat4(&frustum.Orientation, rt);
            cameraComponent.frustum = frustum;
        }
    }
} // namespace Engine::Scene::Systems