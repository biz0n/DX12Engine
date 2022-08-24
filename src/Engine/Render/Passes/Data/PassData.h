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
        Index meshIndex;
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