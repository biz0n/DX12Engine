#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <UI/UIContext.h>

#include <Render/RenderForwards.h>

#include <vector>

namespace Engine::UI::Systems
{
    class UISystem : public Scene::Systems::System
    {
        public:
            UISystem(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Scene::SceneStorage> sceneStorage, SharedPtr<UIContext> uiContext);
            ~UISystem() override;
        public:
            void Process(Scene::SceneRegistry* scene, const Timer& timer) override;

        private:
            SharedPtr<Render::RenderContext> mRenderContext;
            SharedPtr<UIContext> mUiContext;
            SharedPtr<Scene::SceneStorage> mSceneStorage;
    };
}