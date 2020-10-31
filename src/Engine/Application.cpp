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
#include <Scene/SceneLoadingInfo.h>
#include <Types.h>

#include <future>
#include <thread>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/MovingComponent.h>
#include <Scene/Components/CameraComponent.h>

#include <Scene/Systems/WorldTransformSystem.h>
#include <UI/Systems/UISystem.h>
#include <Render/Systems/RenderSystem.h>
#include <Scene/Systems/MovingSystem.h>

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

        mSceneLoadingInfo = MakeShared<Scene::SceneLoadingInfo>();
        mSceneLoadingInfo->scenes = {
            { "Sponza", "Resources\\Scenes\\gltf2\\sponza\\sponza.gltf" },
            { "Corset", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\Corset\\glTF\\Corset.gltf" },
            { "Axis Test", "Resources\\Scenes\\gltf2\\axis.gltf" },
            { "Metal Rough Spheres", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\MetalRoughSpheres\\glTF\\MetalRoughSpheres.gltf" },
            { "Texture Settings Test", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\TextureSettingsTest\\glTF\\TextureSettingsTest.gltf" },
            { "Normal Tangent Mirror Test", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\NormalTangentMirrorTest\\glTF\\NormalTangentMirrorTest.gltf" },
            { "Flight Helmet", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\FlightHelmet\\glTF\\FlightHelmet.gltf" },
            { "Damaged Helmet", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\DamagedHelmet\\glTF\\DamagedHelmet.gltf" },
            { "Orientation Test", "Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\OrientationTest\\glTF\\OrientationTest.gltf" }
        };

        mSceneLoadingInfo->scenePath = mSceneLoadingInfo->scenes["Sponza"];
        mSceneLoadingInfo->loadScene = true;
    }

    void Application::InitScene(UniquePtr<Scene::SceneObject> scene)
    {
        auto camera = scene->registry->view<Scene::Components::CameraComponent>()[0];
        scene->registry->emplace<Scene::Components::MovingComponent>(camera, Scene::Components::MovingComponent());

        scene->AddSystem(MakeUnique<Scene::Systems::WorldTransformSystem>());
        scene->AddSystem(MakeUnique<Scene::Systems::RenderSystem>(mRenderContext));

        scene->AddSystem(MakeUnique<Scene::Systems::UISystem>(mRenderContext, mRenderContext->GetUIContext()));

        scene->AddSystem(MakeUnique<Scene::Systems::MovingSystem>(mKeyboard));

        mScene = std::move(scene);
    }

    void Application::Destroy()
    {
        mRenderContext->WaitForIdle();
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
            if (mSceneLoadingInfo->loadScene)
            {
                mSceneLoadingInfo->loadScene = false;

                mRenderContext->WaitForIdle();
                mScene.reset();

                mSceneLoadingInfo->sceneFuture = std::async(std::launch::async, [this]()
                {    
                    CoInitialize(nullptr);
                    Scene::Loader::SceneLoader loader;
                    return loader.LoadScene(mSceneLoadingInfo->scenePath);
                });
            }

            if (mSceneLoadingInfo->sceneFuture._Is_ready())
            {
                InitScene(mSceneLoadingInfo->sceneFuture.get());
                mSceneLoadingInfo->sceneFuture = {};
            }

            mRenderContext->BeginFrame();

            if (mScene != nullptr)
            {
                mScene->Process(timer);
            }

            mSceneLoadingInfo->DrawSelector();

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
        mRenderContext->WaitForIdle();

        mRenderContext->GetSwapChain()->Resize(width, height);

        mRenderContext->GetUIContext()->Resize(width, height);
    }
} // namespace Engine