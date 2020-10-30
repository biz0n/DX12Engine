#pragma once

#include <Types.h>
#include <Scene/Mesh.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>

#include <vector>
#include <DirectXMath.h>

namespace Engine::Render
{
    struct Mesh
    {
        Scene::Mesh mesh;
        dx::XMMATRIX worldTransform;
    };

    struct Light
    {
        Scene::PunctualLight light;
        dx::XMMATRIX worldTransform;
    };

    struct Camera
    {
        Scene::Camera camera;
        dx::XMMATRIX worldTransform;
    };

    struct RenderingScene
    {
        std::vector<Mesh> meshes;
        std::vector<Light> lights;
        std::vector<Camera> cameras;

        Index activeCamera{0};
    };
}