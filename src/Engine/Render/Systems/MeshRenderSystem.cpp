#include "MeshRenderSystem.h"

#include <Render/MeshRenderer.h>

#include <Scene/SceneObject.h>

namespace Engine::Scene::Systems
{
    MeshRenderSystem::MeshRenderSystem(UniquePtr<Render::MeshRenderer> renderer)
     : mRenderer(std::move(renderer))
     {
     }
    
    MeshRenderSystem::~MeshRenderSystem() = default;

    void MeshRenderSystem::Init(SceneObject *scene)
    {
        mRenderer->Initialize();
    }

    void MeshRenderSystem::Process(SceneObject *scene, const Timer& timer)
    {   
        mRenderer->Render(scene, timer);
    }
}