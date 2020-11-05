#include "RenderSystem.h"


#include <Render/RenderContext.h>
#include <Render/Game.h>

#include <Scene/SceneObject.h>
#include <Scene/Components/WorldTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>

#include <entt/entt.hpp>

namespace Engine::Scene::Systems
{
    RenderSystem::RenderSystem(SharedPtr<RenderContext> renderContext)
     : mRenderContext(renderContext)
     {
     }
    
    RenderSystem::~RenderSystem() = default;

    void RenderSystem::Init(SceneObject *scene)
    {
        mGame = MakeUnique<Game>(mRenderContext);
        mGame->Initialize();
    }

    void RenderSystem::Process(SceneObject *scene, const Timer& timer)
    {   
        mGame->UploadResources(scene);
        mGame->Draw(timer, scene);
    }
}