#pragma once

#include <Types.h>
#include <Scene/Systems/System.h>


namespace Engine
{
    class UIRenderContext;
}

namespace Engine::Scene::Systems
{
    class UISystem : public System
    {
        public:
            UISystem(SharedPtr<UIRenderContext> renderContext);
            ~UISystem() override;
        public:
            void Process(entt::registry *registry, const Timer& timer) override;

        private:
            SharedPtr<UIRenderContext> mRenderContext;
    };
}