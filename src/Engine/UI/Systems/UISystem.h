#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Render/RenderForwards.h>

#include <vector>

namespace Engine::UI::Systems
{
    class UISystem : public Scene::Systems::System
    {
        public:
            UISystem(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Scene::SceneStorage> sceneStorage);
            ~UISystem() override;
        public:
            void Process(Scene::SceneRegistry* scene, const Timer& timer) override;

        private:
            SharedPtr<Render::RenderContext> mRenderContext;

            std::vector<UniquePtr<Engine::UI::ComponentRenderers::ComponentRendererBase>> mComponentRenderers;
    };
}