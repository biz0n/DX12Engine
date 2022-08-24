#include "UploadSceneSystem.h"

#include <Scene/SceneObject.h>
#include <Scene/SceneStorage.h>

namespace Engine::Render::Systems
{
    UploadSceneSystem::UploadSceneSystem(SharedPtr<Scene::SceneStorage> sceneStorage, SharedPtr<Render::RenderContext> renderContext)
        : mSceneStorage{ sceneStorage }, mRenderContext{renderContext}
    {
    }

    UploadSceneSystem::~UploadSceneSystem() = default;

    void UploadSceneSystem::Init(Scene::SceneObject* scene)
    {
    }

    void UploadSceneSystem::Process(Scene::SceneObject* scene, const Timer& timer)
    {
        mSceneStorage->UploadSceneStructures(scene->GetRegistry(), mRenderContext->GetUploadBuffer());
    }
} // namespace Engine::Scene::Systems