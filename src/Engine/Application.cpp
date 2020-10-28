#include "Application.h"

#include <View.h>
#include <RenderContext.h>
#include <SwapChain.h>
#include <Game.h>
#include <Timer.h>
#include <CommandQueue.h>
#include <UIRenderContext.h>
#include <Keyboard.h>

#include <Scene/SceneObject.h>
#include <Scene/Loader/SceneLoader.h>
#include <Types.h>

#include <future>
#include <thread>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/NameComponent.h>

#include <Scene/Systems/WorldTransformSystem.h>
#include <Scene/Systems/UISystem.h>

namespace Engine
{
    Application::Application()
    {
    }

    Application::~Application()
    {
    }

    void Application::Init(View view)
    {
        timer.Reset();

        mKeyboard = MakeShared<Keyboard>();
        mRenderContext = MakeShared<RenderContext>(view);

        mGame = MakeShared<Game>(mRenderContext, mKeyboard);
        mGame->Initialize();

/*
        std::future<UniquePtr<Scene::SceneObject>> sceneFuture = std::async(std::launch::async, [](){

            Scene::Loader::SceneLoader loader;
            return loader.LoadScene("Resources\\Scenes\\gltf2\\sponza\\sponza.gltf");
        });

        mScene = sceneFuture.get();
*/
        auto s = mGame->loadedScene.get();
        s->AddSystem(MakeUnique<Scene::Systems::WorldTransformSystem>());
        s->AddSystem(MakeUnique<Scene::Systems::UISystem>(mRenderContext, mRenderContext->GetUIContext()));
    }

    void Application::Destroy()
    {
        mRenderContext->GetGraphicsCommandQueue()->Flush();
    }

    void Application::Draw()
    {
        timer.Tick();

        if (timer.IsPaused())
        {
            Sleep(16);
        }
        else
        {
            mRenderContext->BeginFrame();

            mGame->Update(timer);
            mGame->Draw(timer);


            mGame->loadedScene.get()->Process(timer);
            //mScene->Process(timer);

            mRenderContext->EndFrame();
        }
    }

    void Application::OnActiveChanged(bool isActive)
    {
        if (isActive)
        {
            timer.Start();
        }
        else
        {
            timer.Stop();
        }
    }

    void Application::OnKeyPressed(KeyEvent event)
    {
        mKeyboard->KeyPressed(event);
    }

    void Application::OnResize(int32 width, int32 height)
    {
        mRenderContext->GetGraphicsCommandQueue()->Flush();

        mRenderContext->GetSwapChain()->Resize(width, height);

        mGame->Resize(width, height);

        mRenderContext->GetUIContext()->Resize(width, height);
    }
} // namespace Engine