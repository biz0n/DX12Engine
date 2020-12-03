#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Scene/Systems/System.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class ForwardPassSystem : public Scene::Systems::System
    {
    public:
        ForwardPassSystem(SharedPtr<Render::Renderer> renderer);
        ~ForwardPassSystem() override;

    public:
        void Init(Scene::SceneObject *scene) override;
        void Process(Scene::SceneObject *scene, const Timer &timer) override;

    private:
        SharedPtr<Render::Renderer> mRenderer;

        UniquePtr<Render::Passes::ForwardPass> mForwardPass;
    };
} // namespace Engine::Scene::Systems