#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Scene/Systems/System.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class RenderSystem : public Scene::Systems::System
    {
    public:
        RenderSystem(SharedPtr<Render::Renderer> renderer);
        ~RenderSystem() override;

    public:
        void Init(Scene::SceneObject *scene) override;
        void Process(Scene::SceneObject *scene, const Timer &timer) override;

    private:
        SharedPtr<Render::Renderer> mRenderer;
    };
} // namespace Engine::Scene::Systems