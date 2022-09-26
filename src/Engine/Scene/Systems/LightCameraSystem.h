#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <Render/RenderForwards.h>

namespace Engine::Scene::Systems
{
    class LightCameraSystem : public System
    {
        public:
            LightCameraSystem(SharedPtr<Render::RenderContext> renderContext);
            ~LightCameraSystem() override;
        public:
            void Process(SceneRegistry *scene, const Timer& timer) override;
        private:
            SharedPtr<Render::RenderContext> mRenderContext;
    };
}