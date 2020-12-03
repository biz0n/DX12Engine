#include "ToneMappingPassSystem.h"

#include <Render/Renderer.h>
#include <Render/Passes/CubePass.h>
#include <Render/Passes/ToneMappingPass.h>

#include <Scene/SceneObject.h>

namespace Engine::Render::Systems
{
    ToneMappingPassSystem::ToneMappingPassSystem(SharedPtr<Render::Renderer> renderer)
        : mRenderer(renderer)
    {
    }

    ToneMappingPassSystem::~ToneMappingPassSystem() = default;

    void ToneMappingPassSystem::Init(Scene::SceneObject *scene)
    {
        mToneMappingPass = MakeUnique<Render::Passes::ToneMappingPass>();
    }

    void ToneMappingPassSystem::Process(Scene::SceneObject *scene, const Timer &timer)
    {
        mRenderer->RegisterRenderPass(mToneMappingPass.get());
    }
} // namespace Engine::Scene::Systems