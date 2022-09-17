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
#include <Render/Passes/ForwardPass.h>
#include <Render/Passes/DepthPass.h>
#include <Render/Passes/BackBufferPass.h>

#include <HAL/SwapChain.h>
#include <HAL/CommandQueue.h>

#include <Bin3D/SceneStorage.h>
#include <Bin3D/Reader/BinaryReader.h>

#include <Scene/SceneObject.h>
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
#include <Render/RenderRequestBuilder.h>

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
            { "Sponza.Bin3D", R"(C:\Users\Maxim\Documents\dev\3d\3DModels\sponza\sponza.bin3d)"},
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

        mRenderer = MakeShared<Render::Renderer>(mRenderContext);

        mSceneLoadingInfo->scenePath = mSceneLoadingInfo->scenes["Sponza.Bin3D"];
        mSceneLoadingInfo->loadScene = true;
    }

    void Application::InitScene(SharedPtr<Bin3D::SceneStorage> sceneDto)
    {
        UniquePtr<Scene::SceneObject> scene = MakeUnique<Scene::SceneObject>();

        Scene::SceneToGPULoader toRegisterLoader{mRenderContext->GetResourceFactory(), mRenderContext->GetResourceCopyManager()};

        Scene::SceneToGPULoader::SceneDataDto sceneData = {};
        sceneData.skyBoxPath = R"(Resources\Scenes\cubemaps\snowcube1024.dds)";
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

        mUiSystem = MakeUnique<UI::Systems::UISystem>(mRenderContext, sceneStorage);
        mRenderGraphSystem = MakeUnique<UI::Systems::RenderGraphSystem>(mRenderer);
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
                    CoInitialize(nullptr);


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

            if (mScene != nullptr)
            {
                mScene->Process(timer);
            }

            mRenderContext->BeginFrame();

            if (mScene != nullptr)
            {
                RenderWork();
            }

            mSceneLoadingInfo->DrawSelector();

            mRenderContext->EndFrame();
        }
    }

    void Application::RenderWork()
    {
        auto renderRequest = Render::RenderRequestBuilder::BuildRequest(mScene.get(), mSceneStorage);

        renderRequest.UploadUniforms(mRenderContext->GetUploadBuffer());

        auto tmp = MakeUnique<Render::Passes::ToneMappingPass>();
        auto bbp = MakeUnique<Render::Passes::BackBufferPass>();
        auto dp = MakeUnique<Render::Passes::DepthPass>();
        auto fp = MakeUnique<Render::Passes::ForwardPass>();
        auto cp = MakeUnique<Render::Passes::CubePass>();

        mRenderer->RegisterRenderPass(tmp.get());
        mRenderer->RegisterRenderPass(bbp.get());
        mRenderer->RegisterRenderPass(dp.get());
        mRenderer->RegisterRenderPass(fp.get());
        mRenderer->RegisterRenderPass(cp.get());

        mRenderer->Render(renderRequest, timer);

        mUiSystem->Process(mScene.get(), timer);
        mRenderGraphSystem->Process(mScene.get(), timer);

        mRenderer->Reset();
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