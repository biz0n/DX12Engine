#include "RenderSystem.h"

#include <Scene/SceneRegistry.h>
#include <Render/Renderer.h>
#include <Render/RenderContext.h>
#include <Render/RenderRequestBuilder.h>

#include <Render/Passes/ForwardPass.h>
#include <Render/Passes/ToneMappingPass.h>
#include <Render/Passes/CubePass.h>
#include <Render/Passes/ForwardPass.h>
#include <Render/Passes/DepthPass.h>
#include <Render/Passes/BackBufferPass.h>

namespace Engine::Render::System
{
    RenderSystem::RenderSystem(SharedPtr<Render::Renderer> renderer, SharedPtr<Render::RenderContext> renderContext, SharedPtr<Scene::SceneStorage> sceneStorage)
        : mRenderer{ renderer }, mRenderContext {renderContext}, mSceneStorage {sceneStorage}
    {

    }

    void RenderSystem::Init(Scene::SceneRegistry* scene)
    {
        mPasses.push_back(MakeUnique<Passes::ToneMappingPass>());
        mPasses.push_back(MakeUnique<Passes::BackBufferPass>());
        mPasses.push_back(MakeUnique<Passes::DepthPass>());
        mPasses.push_back(MakeUnique<Passes::ForwardPass>());
        mPasses.push_back(MakeUnique<Passes::CubePass>());
    }

    void RenderSystem::Process(Scene::SceneRegistry* scene, const Timer& timer)
    {
        auto renderRequest = Render::RenderRequestBuilder::BuildRequest(scene, mSceneStorage);

        renderRequest.UploadUniforms(mRenderContext->GetUploadBuffer());

        mRenderer->Reset();

        for (auto& pass : mPasses)
        {
            mRenderer->RegisterRenderPass(pass.get());
        }

        mRenderer->Render(renderRequest, timer);
    }
}