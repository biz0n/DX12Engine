#pragma once

#include <Types.h>
#include <Scene/Systems/System.h>


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
            void Process(entt::registry *registry, const Timer& timer) override;

        private:
            SharedPtr<UIRenderContext> mUIRenderContext;
            SharedPtr<RenderContext> mRenderContext;
    };
}