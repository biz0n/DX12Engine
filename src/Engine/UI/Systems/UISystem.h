#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <vector>


namespace Engine
{
    class UIRenderContext;
    class RenderContext;
}

namespace Engine::Scene::Systems
{
    class UISystem : public System
    {
        public:
            UISystem(SharedPtr<RenderContext> renderContext, SharedPtr<UIRenderContext> uiRenderContext);
            ~UISystem() override;
        public:
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            SharedPtr<UIRenderContext> mUIRenderContext;
            SharedPtr<RenderContext> mRenderContext;

            std::vector<UniquePtr<Engine::UI::ComponentRenderers::ComponentRendererBase>> mComponentRenderers;
    };
}