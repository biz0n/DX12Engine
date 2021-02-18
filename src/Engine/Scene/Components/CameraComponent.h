#pragma once

#include <Scene/Camera.h>
#include <DirectXCollision.h>

namespace Engine::Scene::Components
{
    struct CameraComponent
    {
        Camera camera;
        dx::XMMATRIX projection;
        dx::XMMATRIX view;
        dx::XMMATRIX viewProjection;
        dx::XMVECTOR eyePosition; 
        dx::BoundingFrustum frustum;
    };

    struct MainCameraComponent
    {
    };
}