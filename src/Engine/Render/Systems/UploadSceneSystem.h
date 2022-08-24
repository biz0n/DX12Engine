#pragma once

#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <Render/RenderContext.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class UploadSceneSystem : public Scene::Systems::System
    {
    public:
        UploadSceneSystem(SharedPtr<Scene::SceneStorage> sceneStorage, SharedPtr<Render::RenderContext> renderContext);
        ~UploadSceneSystem() override;

    public:
        void Init(Scene::SceneObject* scene) override;
        void Process(Scene::SceneObject* scene, const Timer& timer) override;

    private:
        SharedPtr<Scene::SceneStorage> mSceneStorage;
        SharedPtr<Render::RenderContext> mRenderContext;
    };
} // namespace Engine::Scene::Systems