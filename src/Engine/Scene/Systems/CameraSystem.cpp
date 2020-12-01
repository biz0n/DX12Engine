#include "CameraSystem.h"

#include <Render/RenderContext.h>
#include <Render/SwapChain.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/WorldTransformComponent.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    CameraSystem::CameraSystem(SharedPtr<Render::RenderContext> renderContext) : mRenderContext(renderContext)
    {
    }

    CameraSystem::~CameraSystem() = default;

    void CameraSystem::Process(SceneObject *scene, const Timer &timer)
    {
        auto& registry = scene->GetRegistry();

        const auto& view = registry.view<Components::CameraComponent, Components::WorldTransformComponent>();

        const float aspectRatio = mRenderContext->GetSwapChain()->GetWidth() / static_cast<float>(mRenderContext->GetSwapChain()->GetHeight());
        static const dx::XMVECTOR up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        static const dx::XMVECTOR forward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        for (auto &&[entity, cameraComponent, transformComponent] : view.proxy())
        {
            auto& camera = cameraComponent.camera;
            auto& world = transformComponent.transform;
            auto projectionMatrix = camera.GetProjectionMatrix(aspectRatio);

            dx::XMVECTOR sc;
            dx::XMVECTOR rt;
            dx::XMVECTOR tr;
            dx::XMMatrixDecompose(&sc, &rt, &tr, world);

            auto rotationMatrix = dx::XMMatrixRotationQuaternion(rt);
            auto lookAtDirection = dx::XMVector3Transform(
                forward,
                dx::XMMatrixRotationQuaternion(rt));

            using namespace dx;
            auto viewMatrix = dx::XMMatrixLookAtLH(tr, tr + lookAtDirection, up);

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