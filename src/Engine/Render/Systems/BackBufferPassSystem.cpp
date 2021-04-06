#include "BackBufferPassSystem.h"

#include <Render/Renderer.h>
#include <Render/Passes/CubePass.h>
#include <Render/Passes/BackBufferPass.h>

#include <Scene/SceneObject.h>

namespace Engine::Render::Systems
{
    BackBufferPassSystem::BackBufferPassSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(renderer)
    {
    }

    BackBufferPassSystem::~BackBufferPassSystem() = default;

    void BackBufferPassSystem::Init(Scene::SceneObject *scene)
    {
        mBackBufferPass = MakeUnique<Render::Passes::BackBufferPass>();
    }

    void BackBufferPassSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        mRenderer->RegisterRenderPass(mBackBufferPass.get());
    }
} // namespace Engine::Scene::Systems