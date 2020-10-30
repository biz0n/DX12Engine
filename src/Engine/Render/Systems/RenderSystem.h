#pragma once

#include <Types.h>
#include <Scene/Systems/System.h>


namespace Engine
{
    class RenderContext;
}

namespace Engine::Scene::Systems
{
    class RenderSystem : public System
    {
        public:
            RenderSystem(SharedPtr<RenderContext> renderContext);
            ~RenderSystem() override;
        public:
            void Process(entt::registry *registry, const Timer& timer) override;

        private:
            SharedPtr<RenderContext> mRenderContext;
    };
}