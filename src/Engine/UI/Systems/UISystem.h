#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Render/RenderForwards.h>

#include <vector>

namespace Engine::Scene::Systems
{
    class UISystem : public System
    {
        public:
            UISystem(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Render::UIRenderContext> uiRenderContext);
            ~UISystem() override;
        public:
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            SharedPtr<Render::UIRenderContext> mUIRenderContext;
            SharedPtr<Render::RenderContext> mRenderContext;

            std::vector<UniquePtr<Engine::UI::ComponentRenderers::ComponentRendererBase>> mComponentRenderers;
    };
}