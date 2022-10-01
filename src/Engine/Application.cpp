#include "Application.h"

#include <View.h>
#include <Types.h>
#include <Timer.h>

#include <IO/Keyboard.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>
#include <Render/Renderer.h>
#include <Render/Systems/RenderSystem.h>

#include <HAL/SwapChain.h>
#include <HAL/CommandQueue.h>

#include <Bin3D/Scene.h>
#include <Bin3D/Reader/BinaryReader.h>

#include <Scene/SceneRegistry.h>
#include <Scene/SceneLoadingInfo.h>
#include <Scene/SceneToGPULoader.h>
#include <Scene/SceneStorage.h>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/MovingComponent.h>
#include <Scene/Components/CameraComponent.h>

#include <Scene/Systems/WorldTransformSystem.h>
#include <Scene/Systems/MovingSystem.h>
#include <Scene/Systems/CameraSystem.h>
#include <Scene/Systems/LightCameraSystem.h>
#include <UI/Systems/UISystem.h>
#include <UI/Systems/RenderGraphSystem.h>

#include <PathResolver.h>

#include <Exceptions.h>

#include <future>
#include <thread>

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
        mRenderContext = MakeShared<Render::RenderContext>(view);

        mSceneLoadingInfo = MakeShared<Scene::SceneLoadingInfo>();
        mSceneLoadingInfo->scenes = {
            { "Sponza.Bin3D",  PathResolver::GetResourcePath(R"(3DModels\sponza\model.bin3d)")},
            { "SponzaNew.Bin3D", PathResolver::GetResourcePath(R"(3DModels\sponza_new\model.bin3d)")},
            { "DamagedHelmet.Bin3D", PathResolver::GetResourcePath(R"(3DModels\DamagedHelmet\model.bin3d)")},
            { "Cube.Bin3D", PathResolver::GetResourcePath(R"(3DModels\cube\model.bin3d)")},
            { "FlightHelmet.Bin3D", PathResolver::GetResourcePath(R"(3DModels\FlightHelmet\model.bin3d)")},
            { "Corset.Bin3D", PathResolver::GetResourcePath(R"(Corset\model.bin3d)")},
            { "Avocado.Bin3D", PathResolver::GetResourcePath(R"(3DModels\Avocado\model.bin3d)")},
            { "BoomBoxWithAxes.Bin3D", PathResolver::GetResourcePath(R"(3DModels\BoomBoxWithAxes\model.bin3d)")},
            { "BoomBox.Bin3D", PathResolver::GetResourcePath(R"(3DModels\BoomBox\model.bin3d)")},
        };

        mRenderer = MakeShared<Render::Renderer>(mRenderContext);

        mSceneLoadingInfo->scenePath = mSceneLoadingInfo->scenes["Sponza.Bin3D"];
        mSceneLoadingInfo->loadScene = true;
    }

    void Application::InitScene(SharedPtr<Bin3D::Scene> sceneDto)
    {
        UniquePtr<Scene::SceneRegistry> scene = MakeUnique<Scene::SceneRegistry>();

        Scene::SceneToGPULoader toRegisterLoader{mRenderContext->GetResourceFactory(), mRenderContext->GetResourceCopyManager()};

        Scene::SceneToGPULoader::SceneDataDto sceneData = {};
        sceneData.skyBoxPath = PathResolver::GetResourcePath(R"(Cubemaps\old_outdoor_theater_4k.dds)").string();
        SharedPtr<Scene::SceneStorage> sceneStorage = toRegisterLoader.LoadSceneToGPU(scene->GetRegistry(), sceneDto, sceneData);

        auto& registry = scene->GetRegistry();
        auto [cameraEntity, camera] = scene->GetMainCamera();
        if (cameraEntity != entt::null)
        {
            registry.emplace<Scene::Components::MovingComponent>(cameraEntity, Scene::Components::MovingComponent());
        }

        scene->AddSystem(MakeUnique<Scene::Systems::WorldTransformSystem>());

        scene->AddSystem(MakeUnique<Scene::Systems::CameraSystem>(mRenderContext));
        scene->AddSystem(MakeUnique<Scene::Systems::LightCameraSystem>(mRenderContext));

        scene->AddSystem(MakeUnique<Scene::Systems::MovingSystem>(mKeyboard));

        scene->AddSystem(MakeUnique<Render::System::RenderSystem>(mRenderer, mRenderContext, sceneStorage));

        scene->AddSystem(MakeUnique<UI::Systems::UISystem>(mRenderContext, sceneStorage));
        scene->AddSystem(MakeUnique<UI::Systems::RenderGraphSystem>(mRenderer));

        mScene = std::move(scene);
        mSceneStorage = sceneStorage;
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
                    Bin3D::Reader::BinaryReader reader;

                    auto sceneDto = reader.ReadScene(mSceneLoadingInfo->scenePath);

                    return sceneDto;
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