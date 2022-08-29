#pragma once

#include <Types.h>

#include <Render/RenderRequest.h>
#include <Scene/SceneForwards.h>
#include <Render/Shaders/ShaderTypes.h>

#include <vector>

namespace Engine::Render
{
    class RenderRequestBuilder
    {
    public:
        static RenderRequest BuildRequest(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage);

    private:
        static MeshPack PrepareMeshes(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage);
        static std::vector<Shader::MaterialUniform> PrepareMaterials(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage);
        static std::vector<Shader::LightUniform> PrepareLights(const Engine::Scene::SceneObject* sceneObject, SharedPtr<Engine::Scene::SceneStorage> sceneStorage);
        static RenderCamera PrepareRenderCamera(const Engine::Scene::SceneObject* sceneObject);
        static std::vector<RenderCamera> PrepareShadowCameras(const Engine::Scene::SceneObject* sceneObject);

    };
}