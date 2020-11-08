#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>


namespace Engine::Render
{
    class SceneRenderer;
}

namespace Engine::Scene::Systems
{
    class RenderSystem : public System
    {
        public:
            RenderSystem(UniquePtr<Render::SceneRenderer> renderer);
            ~RenderSystem() override;
        public:
            void Init(SceneObject *scene) override;
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            UniquePtr<Render::SceneRenderer> mRenderer;
    };
}