#pragma once

#include <Render/RenderForwards.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>

#include <vector>

namespace Engine::Render::System
{
    class RenderSystem : public Scene::Systems::System
    {
    public:
        RenderSystem(SharedPtr<Render::Renderer> renderer, SharedPtr<Render::RenderContext> renderContext, SharedPtr<Scene::SceneStorage> sceneStorage);
    public:
        void Init(Scene::SceneRegistry* scene) override;
        void Process(Scene::SceneRegistry* scene, const Timer& timer) override;
    private:
        SharedPtr<Render::Renderer> mRenderer;
        SharedPtr<Render::RenderContext> mRenderContext;
        SharedPtr<Scene::SceneStorage> mSceneStorage;
        std::vector<UniquePtr<RenderPassBase>> mPasses;
    };
}

