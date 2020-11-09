#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>


namespace Engine::Render
{
    class MeshRenderer;
}

namespace Engine::Scene::Systems
{
    class MeshRenderSystem : public System
    {
        public:
            MeshRenderSystem(UniquePtr<Render::MeshRenderer> renderer);
            ~MeshRenderSystem() override;
        public:
            void Init(SceneObject *scene) override;
            void Process(SceneObject *scene, const Timer& timer) override;

        private:
            UniquePtr<Render::MeshRenderer> mRenderer;
    };
}