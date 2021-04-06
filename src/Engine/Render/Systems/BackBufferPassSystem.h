#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <Scene/Systems/System.h>
#include <Render/RenderForwards.h>
#include <Timer.h>

namespace Engine::Render::Systems
{
    class BackBufferPassSystem : public Scene::Systems::System
    {
    public:
        BackBufferPassSystem(SharedPtr<Render::Renderer> renderer);
        ~BackBufferPassSystem() override;

    public:
        void Init(Scene::SceneObject *scene) override;
        void Process(Scene::SceneObject *scene, const Timer &timer) override;

    private:
        SharedPtr<Render::Renderer> mRenderer;

        UniquePtr<Render::RenderPassBase> mBackBufferPass;
    };
} // namespace Engine::Scene::Systems