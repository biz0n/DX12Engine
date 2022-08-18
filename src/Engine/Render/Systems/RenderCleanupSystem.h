#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Scene/Systems/System.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class RenderCleanupSystem : public Scene::Systems::System
    {
    public:
        RenderCleanupSystem(SharedPtr<Render::Renderer> renderer);
        ~RenderCleanupSystem() override;

    public:
        void Init(Scene::SceneObject* scene) override;
        void Process(Scene::SceneObject* scene, const Timer& timer) override;

    private:
        SharedPtr<Render::Renderer> mRenderer;
    };
} // namespace Engine::Scene::Systems