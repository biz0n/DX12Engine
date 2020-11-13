#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>


namespace Engine::Render
{
    class Renderer;
}

namespace Engine::Scene::Systems
{
    class RenderSystem : public System
    {
        public:
            RenderSystem(UniquePtr<Render::Renderer> renderer);
            ~RenderSystem() override;
        public:
            void Init(SceneObject *scene) override;
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            UniquePtr<Render::Renderer> mRenderer;
    };
}