#include "RenderSystem.h"

#include <Render/Renderer.h>

#include <Scene/SceneObject.h>

namespace Engine::Scene::Systems
{
    RenderSystem::RenderSystem(UniquePtr<Render::Renderer> renderer)
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