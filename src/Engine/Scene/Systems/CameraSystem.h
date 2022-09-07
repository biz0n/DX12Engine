#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <Render/RenderForwards.h>

namespace Engine::Scene::Systems
{
    class CameraSystem : public System
    {
        public:
            CameraSystem(SharedPtr<Render::RenderContext> renderContext);
            ~CameraSystem() override;
        public:
            void Process(SceneObject *scene, const Timer& timer) override;
        private:
            SharedPtr<Render::RenderContext> mRenderContext;

    };
}