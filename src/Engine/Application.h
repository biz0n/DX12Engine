#pragma once

#include <Types.h>
#include <IO/KeyEvent.h>
#include <Timer.h>

#include <Bin3D/Forwards.h>
#include <Render/RenderForwards.h>
#include <UI/Systems/UISystem.h>
#include <UI/Systems/RenderGraphSystem.h>

namespace Engine
{
    class Keyboard;
    struct View;

    class Application
    {
    public:
        Application();
        ~Application();

        void Init(View view);
        void Destroy();

        void Draw();

        void RenderWork();

        void OnActiveChanged(bool isActive);
        void OnKeyPressed(KeyEvent event);
        void OnResize(int32 width, int32 height);

    private:
        void InitScene(SharedPtr<Bin3D::Scene> scene);

    private:
        SharedPtr<Render::RenderContext> mRenderContext;
        SharedPtr<Render::Renderer> mRenderer;
        SharedPtr<Keyboard> mKeyboard;
        SharedPtr<Scene::SceneLoadingInfo> mSceneLoadingInfo;

        UniquePtr<Scene::SceneRegistry> mScene;
        SharedPtr<Scene::SceneStorage> mSceneStorage;

        UniquePtr<UI::Systems::UISystem> mUiSystem;
        UniquePtr<UI::Systems::RenderGraphSystem> mRenderGraphSystem;

    private:
        Timer timer;
    };
} // namespace Engine