#include "Application.h"

#include <View.h>
#include <Types.h>
#include <Timer.h>

#include <IO/Keyboard.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>
#include <Render/Renderer.h>

#include <Render/Passes/ForwardPass.h>
#include <Render/Passes/ToneMappingPass.h>
#include <Render/Passes/CubePass.h>

#include <HAL/SwapChain.h>
#include <HAL/CommandQueue.h>

#include <Scene/Loader/SceneDto.h>
#include <Scene/SceneObject.h>
#include <Scene/SceneLoadingInfo.h>
#include <Scene/Loader/SceneLoader.h>
#include <Scene/SceneToRegistryLoader.h>

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
#include <Render/Systems/RenderSystem.h>
#include <Render/Systems/DepthPassSystem.h>
#include <Render/Systems/ForwardPassSystem.h>
#include <Render/Systems/CubePassSystem.h>
#include <Render/Systems/ToneMappingPassSystem.h>
#include <Render/Systems/BackBufferPassSystem.h>

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
            { "Sponza", R"(Resources\Scenes\gltf2\sponza\sponza.gltf)" },
            { "Corset", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\Corset\glTF\Corset.gltf)" },
            { "Axis Test", R"(Resources\Scenes\gltf2\axis.gltf)" },
            { "Metal Rough Spheres", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\MetalRoughSpheres\glTF\MetalRoughSpheres.gltf)" },
            { "Texture Settings Test", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\TextureSettingsTest\glTF\TextureSettingsTest.gltf)" },
            { "Normal Tangent Mirror Test", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\NormalTangentMirrorTest\glTF\NormalTangentMirrorTest.gltf)" },
            { "Flight Helmet", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\FlightHelmet\glTF\FlightHelmet.gltf)" },
            { "Damaged Helmet", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\DamagedHelmet\glTF\DamagedHelmet.gltf)" },
            { "Orientation Test", R"(Resources\Scenes\glTF-Sample-Models-master\2.0\OrientationTest\glTF\OrientationTest.gltf)" },
            { "s_test", R"(Resources\Scenes\shadow\test1.gltf)" },
            { "sPONZA_NEW_CURTAINS", R"(C:\Users\Maxim\Downloads\PKG_A_Curtains\PKG_A_Curtains\NewSponza_Curtains_glTF.gltf)"},
             { "sPONZA_NEW", R"(C:\Users\Maxim\Downloads\Main\Main\NewSponza_Main_Blender_glTF - Copy.gltf)"}
        };

        mSceneLoadingInfo->scenePath = mSceneLoadingInfo->scenes["Sponza"];
        mSceneLoadingInfo->loadScene = true;
    }

    void Application::InitScene(Scene::Loader::SceneDto sceneDto)
    {
        UniquePtr<Scene::SceneObject> scene = MakeUnique<Scene::SceneObject>();

        Scene::SceneToRegisterLoader toRegisterLoader{mRenderContext->GetResourceFactory(), mRenderContext->GetResourceCopyManager()};
        toRegisterLoader.LoadSceneToRegistry(scene->GetRegistry(), sceneDto);
        toRegisterLoader.AddCubeMapToScene(scene->GetRegistry(), R"(Resources\Scenes\cubemaps\snowcube1024.dds)");

        auto renderer = MakeShared<Render::Renderer>(mRenderContext);

        auto& registry = scene->GetRegistry();
        auto [cameraEntity, camera] = scene->GetMainCamera();
        if (cameraEntity != entt::null)
        {
            registry.emplace<Scene::Components::MovingComponent>(cameraEntity, Scene::Components::MovingComponent());
        }

        scene->AddSystem(MakeUnique<Scene::Systems::WorldTransformSystem>());

        scene->AddSystem(MakeUnique<Scene::Systems::CameraSystem>(mRenderContext));
        scene->AddSystem(MakeUnique<Scene::Systems::LightCameraSystem>(mRenderContext));

        scene->AddSystem(MakeUnique<Render::Systems::DepthPassSystem>(renderer));
        scene->AddSystem(MakeUnique<Render::Systems::ForwardPassSystem>(renderer));
        scene->AddSystem(MakeUnique<Render::Systems::CubePassSystem>(renderer));
        scene->AddSystem(MakeUnique<Render::Systems::ToneMappingPassSystem>(renderer));
        scene->AddSystem(MakeUnique<Render::Systems::BackBufferPassSystem>(renderer));

        scene->AddSystem(MakeUnique<Render::Systems::RenderSystem>(renderer));

        scene->AddSystem(MakeUnique<UI::Systems::UISystem>(mRenderContext));
        scene->AddSystem(MakeUnique<UI::Systems::RenderGraphSystem>());

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


                    auto sceneDto = loader.LoadScene(mSceneLoadingInfo->scenePath);


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