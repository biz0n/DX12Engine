#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>


namespace Engine
{
    class RenderContext;
    class Game;
}

namespace Engine::Scene::Systems
{
    class RenderSystem : public System
    {
        public:
            RenderSystem(SharedPtr<RenderContext> renderContext);
            ~RenderSystem() override;
        public:
            void Init(SceneObject *scene) override;
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            SharedPtr<RenderContext> mRenderContext;
            UniquePtr<Game> mGame;
    };
}