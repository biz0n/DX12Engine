#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <Render/RenderForwards.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class DepthPassSystem : public Scene::Systems::System
    {
    public:
        DepthPassSystem(SharedPtr<Render::Renderer> renderer);
        ~DepthPassSystem() override;

    public:
        void Init(Scene::SceneObject *scene) override;
        void Process(Scene::SceneObject *scene, const Timer &timer) override;

    private:
        SharedPtr<Render::Renderer> mRenderer;

        UniquePtr<Render::Passes::DepthPass> mDepthPass;
    };
} // namespace Engine::Scene::Systems