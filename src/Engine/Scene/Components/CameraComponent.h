#pragma once

#include <Bin3D/Camera.h>
#include <DirectXCollision.h>

namespace Engine::Scene::Components
{
    struct CameraComponent
    {
        Bin3D::Camera camera;
        dx::XMMATRIX projection;
        dx::XMMATRIX view;
        dx::XMMATRIX viewProjection;
        dx::XMVECTOR eyePosition; 
        dx::BoundingFrustum frustum;

        dx::XMMATRIX GetProjectionMatrix(float32 width, float32 height) const
        {
            if (camera.Type == Bin3D::CameraType::Perspective)
            {
                return dx::XMMatrixPerspectiveFovLH(camera.FoV, width / height, camera.NearPlane, camera.FarPlane);
            }
            else
            {
                return dx::XMMatrixOrthographicLH(width * camera.OrthographicXMag, height * camera.OrthographicYMag, camera.NearPlane, camera.FarPlane);
            }
        }
    };

    struct MainCameraComponent
    {
    };
}