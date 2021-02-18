#pragma once

#include <Types.h>
#include <Scene/Mesh.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>
#include <Scene/CubeMap.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace Engine::Render::Passes
{
    struct MeshData
    {
        Scene::Mesh mesh;
        dx::XMMATRIX worldTransform;
    };

    struct LightData
    {
        Scene::PunctualLight light;
        dx::XMMATRIX worldTransform;
    };

    struct CameraData
    {
        dx::XMMATRIX projection;
        dx::XMMATRIX view;
        dx::XMMATRIX viewProjection;
        dx::XMVECTOR eyePosition; 
    };

    struct CubeData
    {
        Scene::CubeMap cubeMap;
    };
}