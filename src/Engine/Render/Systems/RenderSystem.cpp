#include "RenderSystem.h"

#include <Render/SceneRenderer.h>

#include <Scene/SceneObject.h>

namespace Engine::Scene::Systems
{
    RenderSystem::RenderSystem(UniquePtr<Render::SceneRenderer> renderer)
     : mRenderer(std::move(renderer))
     {
     }
    
    RenderSystem::~RenderSystem() = default;

    void RenderSystem::Init(SceneObject *scene)
    {
        mRenderer->Initialize();
    }

    void RenderSystem::Process(SceneObject *scene, const Timer& timer)
    {   
        mRenderer->Render(scene, timer);
    }
}