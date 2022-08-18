#include "RenderCleanupSystem.h"

#include <Render/Renderer.h>

#include <Scene/SceneObject.h>

namespace Engine::Render::Systems
{
    RenderCleanupSystem::RenderCleanupSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(std::move(renderer))
    {
    }

    RenderCleanupSystem::~RenderCleanupSystem() = default;

    void RenderCleanupSystem::Init(Scene::SceneObject* scene)
    {
    }

    void RenderCleanupSystem::Process(Scene::SceneObject* scene, const Timer& timer)
    {
        mRenderer->Reset();
    }
} // namespace Engine::Scene::Systems