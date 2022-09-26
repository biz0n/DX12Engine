#include "LightCameraSystem.h"

#include <EngineConfig.h>

#include <Render/RenderContext.h>
#include <HAL/SwapChain.h>

#include <Bin3D/Camera.h>
#include <Bin3D/PunctualLight.h>
#include <Scene/SceneRegistry.h>
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

    void LightCameraSystem::Process(SceneRegistry *scene, const Timer &timer)
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

            dx::XMVECTOR direction = dx::XMVector4Transform(forward, world);

            auto lMax = std::max(light.Color.x, std::max(light.Color.y, light.Color.z)) * light.Intensity;
            auto maxDistance = (
                -light.LinearAttenuation + std::sqrtf(
                        light.LinearAttenuation * light.LinearAttenuation - 4 * light.QuadraticAttenuation * (light.ConstantAttenuation - (256.0 / 5.0) * lMax)
                    )
                )
                / (2 * (light.QuadraticAttenuation + 0.0001f));

            maxDistance = std::max(maxDistance, 0.1f);

            switch(light.LightType)
            {
                case Bin3D::LightType::SpotLight:
                {
                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    camera.Type = Bin3D::CameraType::Perspective;
                    camera.NearPlane = 0.0001f;
                    camera.FarPlane = maxDistance;
                    camera.FoV = light.OuterConeAngle;
                    projectionMatrix = cameraComponent.GetProjectionMatrix(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);
                }
                break;
                case Bin3D::LightType::PointLight: // not supported for now, but light will show only forward camera
                    viewMatrix = dx::XMMatrixLookToLH(tr, forward, up);
                    camera.Type = Bin3D::CameraType::Perspective;
                    camera.NearPlane = 0.0001f;
                    camera.FarPlane = maxDistance;
                    camera.FoV = dx::XMConvertToRadians(90);
                    projectionMatrix = cameraComponent.GetProjectionMatrix(EngineConfig::ShadowWidth, EngineConfig::ShadowHeight);
                break;
                case Bin3D::LightType::DirectionalLight:
                {
                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    dx::XMVECTOR D;
                    const auto inverseView = dx::XMMatrixInverse(&D, viewMatrix);
                    dx::XMMatrixDecompose(&unused, &rt, &unused, inverseView);
                    camera.Type = Bin3D::CameraType::Orthographic;

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

                    camera.NearPlane = nearPlane;
                    camera.FarPlane = farPlane;
                    camera.OrthographicXMag = 1;
                    camera.OrthographicYMag = 1;

                    viewMatrix = dx::XMMatrixLookToLH(tr, direction, up);
                    const auto maxDimension = 2 * std::max(boxWidth, boxHeight);
                    projectionMatrix = cameraComponent.GetProjectionMatrix(maxDimension, maxDimension);
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