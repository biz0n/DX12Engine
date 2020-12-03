#include "RenderSystem.h"

#include <Render/Renderer.h>

#include <Scene/SceneObject.h>

namespace Engine::Render::Systems
{
    RenderSystem::RenderSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(std::move(renderer))
    {
    }

    RenderSystem::~RenderSystem() = default;

    void RenderSystem::Init(Scene::SceneObject *scene)
    {
        mRenderer->Initialize();
    }

    void RenderSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        mRenderer->Render(scene, timer);
    }
} // namespace Engine::Scene::Systems